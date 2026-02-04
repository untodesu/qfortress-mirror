#include "core/pch.hh"

#include "core/level/loader.hh"

#include "core/buffer.hh"
#include "core/entity/components.hh"
#include "core/exceptions.hh"
#include "core/level/bnode.hh"
#include "core/level/level.hh"
#include "core/level/vertex.hh"
#include "core/utils/physfs.hh"
#include "core/utils/string.hh"

constexpr static std::uint8_t MAGIC_BYTE_0 = 'Q';
constexpr static std::uint8_t MAGIC_BYTE_1 = 'F';
constexpr static std::uint8_t MAGIC_BYTE_2 = 'L';
constexpr static std::uint8_t MAGIC_BYTE_3 = 'V';

constexpr static std::uint32_t QFLV_VERSION = 1;

constexpr static std::uint32_t LUMP_GEO = 1; ///< Geometry nodes
constexpr static std::uint32_t LUMP_PVS = 2; ///< Potentially visible set
constexpr static std::uint32_t LUMP_ENT = 3; ///< Entity data as a JSON string
constexpr static std::uint32_t LUMP_RAD = 4; ///< Lightmaps
constexpr static std::uint32_t LUMP_IDX = 5; ///< Index soup to be uploaded directly into an IBO
constexpr static std::uint32_t LUMP_VTX = 6; ///< Vertex soup to be uploaded directly into a VBO

struct ProtoBNode final {
    float plane_coefs[4];
    std::int32_t leaf_index { -1 };
    std::int32_t front { -1 };
    std::int32_t back { -1 };
    std::string material;
    std::int32_t ebo_offset { -1 };
    std::int32_t ebo_count { -1 };
};

static void read_geometry(ReadBuffer& buffer)
{
    auto nodecnt = buffer.read<std::uint32_t>();

    std::vector<ProtoBNode> protonodes;
    protonodes.reserve(nodecnt);

    qf::throw_if<std::runtime_error>(nodecnt == 0, "empty geometry lump");
    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

    for(std::uint32_t i = 0; i < nodecnt; ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        ProtoBNode protonode;
        protonode.plane_coefs[0] = buffer.read<float>();
        protonode.plane_coefs[1] = buffer.read<float>();
        protonode.plane_coefs[1] = buffer.read<float>();
        protonode.plane_coefs[2] = buffer.read<float>();
        protonode.leaf_index = buffer.read<std::int32_t>();
        protonode.front = buffer.read<std::int32_t>();
        protonode.back = buffer.read<std::int32_t>();
        protonode.material = buffer.read<std::string>();
        protonode.ebo_offset = buffer.read<std::int32_t>();
        protonode.ebo_count = buffer.read<std::int32_t>();

        protonodes.emplace_back(std::move(protonode));
    }

    std::vector<std::shared_ptr<BNode>> nodes;
    nodes.reserve(nodecnt);

    for(std::uint32_t i = 0; i < nodecnt; ++i) {
        auto node = std::make_shared<BNode>();
        auto& protonode = protonodes[i];

        if(protonode.leaf_index > 0) {
            BNode::Leaf leaf;
            leaf.index = protonode.leaf_index;
            leaf.ebo_offset = protonode.ebo_offset;
            leaf.ebo_count = protonode.ebo_count;
            leaf.material = protonode.material;

            node->data = std::move(leaf);
        }
        else {
            BNode::Internal internal;
            internal.plane = Eigen::Hyperplane<float, 3>(Eigen::Map<Eigen::Vector3f>(&protonode.plane_coefs[0]), protonode.plane_coefs[3]);
            internal.front = nullptr;
            internal.back = nullptr;

            node->data = internal;
        }

        nodes.emplace_back(std::move(node));
    }

    for(std::uint32_t i = 0; i < nodecnt; ++i) {
        auto& node = nodes[i];
        auto& protonode = protonodes[i];

        if(auto internal = std::get_if<BNode::Internal>(&node->data)) {
            auto front_index = protonode.front;
            auto back_index = protonode.back;

            qf::throw_if<std::runtime_error>(front_index < 0 || front_index >= nodes.size(), "invalid front child index");
            qf::throw_if<std::runtime_error>(back_index < 0 || back_index >= nodes.size(), "invalid back child index");

            internal->front = nodes[front_index];
            internal->back = nodes[back_index];
        }
    }

    level::root = nodes[0];
}

static void read_pvs(ReadBuffer& buffer)
{
    auto nodecnt = buffer.read<std::uint32_t>();

    qf::throw_if<std::runtime_error>(nodecnt == 0, "empty PVS lump");
    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

    std::vector<std::uint32_t> pvsentry;
    pvsentry.reserve(nodecnt);

    for(std::uint32_t i = 0; i < nodecnt; ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        pvsentry.clear();

        for(std::uint32_t j = 0; j < nodecnt; ++j) {
            qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

            pvsentry.push_back(buffer.read<std::uint32_t>());
        }

        level::PVS.push_back(pvsentry);
    }

    qf::throw_if_not<std::runtime_error>(nodecnt == level::PVS.size(), "PVS size mismatch");
}

static void read_entities(ReadBuffer& buffer)
{
    std::string source;
    source.resize(buffer.read<std::uint32_t>());
    buffer.read(source.data(), source.size());

    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

    const auto jsonv = json_parse_string(source.c_str());
    qf::throw_if_not<std::runtime_error>(jsonv, "json syntax error");

    const auto json = json_value_get_object(jsonv);
    qf::throw_if_not<std::runtime_error>(json, "root json value is not an Object");

    auto count = json_object_get_count(json);

    for(std::size_t i = 0; i < count; ++i) {
        auto id_string = json_object_get_name(json, i);
        auto value = json_object_get_value_at(json, i);

        assert(id_string);
        assert(value);

        std::uint64_t entity_64;
        std::string_view id_view(id_string, std::strlen(id_string));

        auto check = std::from_chars(id_view.data(), id_view.data() + id_view.size(), entity_64);
        qf::throw_if_not<std::runtime_error>(check.ec == std::errc(), "invalid entity id");

        auto entity_request = static_cast<entt::entity>(entity_64);
        auto entity = level::registry.create(entity_request);
        qf::throw_if_not<std::runtime_error>(entity_request == entity, "entity id mismatch");

        components::deserialize_entity(level::registry, entity, value);
    }

    json_value_free(jsonv);
}

static void read_lightmaps(ReadBuffer& buffer)
{
    // empty
}

static void read_indices(ReadBuffer& buffer)
{
    level::indices.resize(buffer.read<std::uint32_t>());

    for(std::size_t i = 0; i < level::indices.size(); ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        level::indices[i] = buffer.read<std::uint32_t>();
    }
}

static void read_vertices(ReadBuffer& buffer)
{
    level::vertices.resize(buffer.read<std::uint32_t>());

    for(std::size_t i = 0; i < level::vertices.size(); ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        LevelVertex vertex;

        vertex.position.x() = buffer.read<float>();
        vertex.position.y() = buffer.read<float>();
        vertex.position.z() = buffer.read<float>();
        assert(vertex.position.allFinite());

        vertex.normal.x() = buffer.read<float>();
        vertex.normal.y() = buffer.read<float>();
        vertex.normal.z() = buffer.read<float>();
        assert(vertex.normal.allFinite());

        vertex.tangent.x() = buffer.read<float>();
        vertex.tangent.y() = buffer.read<float>();
        vertex.tangent.z() = buffer.read<float>();
        vertex.tangent.w() = buffer.read<float>();
        assert(vertex.tangent.allFinite());

        vertex.texcoord.x() = buffer.read<float>();
        vertex.texcoord.y() = buffer.read<float>();
        assert(vertex.texcoord.allFinite());

        vertex.lightmap.x() = buffer.read<float>();
        vertex.lightmap.y() = buffer.read<float>();
        assert(vertex.lightmap.allFinite());

        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");
    }
}

static void write_geometry(WriteBuffer& buffer)
{
    std::vector<const BNode*> nodes;
    std::unordered_map<const BNode*, std::int32_t> indices;
    level::flatten_bsp(nodes, indices);

    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(nodes.size()));

    for(const auto node : nodes) {
        if(const auto leaf = std::get_if<BNode::Leaf>(&node->data)) {
            buffer.write<float>(std::numeric_limits<float>::quiet_NaN());
            buffer.write<float>(std::numeric_limits<float>::quiet_NaN());
            buffer.write<float>(std::numeric_limits<float>::quiet_NaN());
            buffer.write<float>(std::numeric_limits<float>::quiet_NaN());

            buffer.write<std::int32_t>(leaf->index);
            buffer.write<std::int32_t>(-1);
            buffer.write<std::int32_t>(-1);

            buffer.write<std::string_view>(leaf->material);
            buffer.write<std::int32_t>(leaf->ebo_offset);
            buffer.write<std::int32_t>(leaf->ebo_count);
        }
        else if(const auto internal = std::get_if<BNode::Internal>(&node->data)) {
            buffer.write<float>(internal->plane.coeffs()[0]);
            buffer.write<float>(internal->plane.coeffs()[1]);
            buffer.write<float>(internal->plane.coeffs()[2]);
            buffer.write<float>(internal->plane.coeffs()[3]);

            buffer.write<std::int32_t>(-1);
            buffer.write<std::int32_t>(internal->front ? indices.at(internal->front.get()) : -1);
            buffer.write<std::int32_t>(internal->back ? indices.at(internal->back.get()) : -1);

            buffer.write<std::string_view>(std::string_view());
            buffer.write<std::int32_t>(-1);
            buffer.write<std::int32_t>(-1);
        }
    }
}

static void write_pvs(WriteBuffer& buffer)
{
    auto nodecnt = static_cast<std::uint32_t>(level::PVS.size());

    buffer.write<std::uint32_t>(nodecnt);

    for(const auto& row : level::PVS) {
        for(std::size_t i = 0; i < nodecnt; ++i) {
            if(i < row.size()) {
                buffer.write<std::uint32_t>(row[i]);
            }
            else {
                buffer.write<std::uint32_t>(0);
            }
        }
    }
}

static void write_entities(WriteBuffer& buffer)
{
    auto jsonv = json_value_init_object();
    auto json = json_value_get_object(jsonv);
    assert(jsonv);

    auto view = level::registry.view<entt::entity>();

    for(auto [entity] : view.each()) {
        auto value = components::serialize_entity(level::registry, entity);
        assert(value);

        auto entity_64 = static_cast<std::uint64_t>(entity);
        auto id_string = std::to_string(entity_64);

        json_object_set_value(json, id_string.c_str(), value);
    }

    std::string source;
    source.assign(json_serialize_to_string(jsonv));

    json_value_free(jsonv);

    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(source.size()));
    buffer.write(source.data(), source.size());
}

static void write_lightmaps(WriteBuffer& buffer)
{
    // empty
}

static void write_indices(WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(level::indices.size()));

    for(auto index : level::indices) {
        buffer.write<std::uint32_t>(index);
    }
}

static void write_vertices(WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(level::vertices.size()));

    for(const auto& vertex : level::vertices) {
        buffer.write<float>(vertex.position.x());
        buffer.write<float>(vertex.position.y());
        buffer.write<float>(vertex.position.z());

        buffer.write<float>(vertex.normal.x());
        buffer.write<float>(vertex.normal.y());
        buffer.write<float>(vertex.normal.z());

        buffer.write<float>(vertex.tangent.x());
        buffer.write<float>(vertex.tangent.y());
        buffer.write<float>(vertex.tangent.z());
        buffer.write<float>(vertex.tangent.w());

        buffer.write<float>(vertex.texcoord.x());
        buffer.write<float>(vertex.texcoord.y());

        buffer.write<float>(vertex.lightmap.x());
        buffer.write<float>(vertex.lightmap.y());
    }
}

void level::load(std::string_view path)
{
    auto path_unfucked = std::string(path);
    auto file = PHYSFS_openRead(path_unfucked.c_str());

    qf::throw_if_not<std::runtime_error>(file, utils::physfs_error());

    ReadBuffer buffer;
    buffer.reset(file);
    PHYSFS_close(file);

    auto magic_0 = buffer.read<std::uint8_t>();
    auto magic_1 = buffer.read<std::uint8_t>();
    auto magic_2 = buffer.read<std::uint8_t>();
    auto magic_3 = buffer.read<std::uint8_t>();

    auto signature_valid = true;
    signature_valid = signature_valid && magic_0 == MAGIC_BYTE_0;
    signature_valid = signature_valid && magic_1 == MAGIC_BYTE_1;
    signature_valid = signature_valid && magic_2 == MAGIC_BYTE_2;
    signature_valid = signature_valid && magic_3 == MAGIC_BYTE_3;

    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");
    qf::throw_if_not<std::runtime_error>(signature_valid, "invalid file signature");

    auto version = buffer.read<std::uint32_t>();
    auto lumpcnt = buffer.read<std::uint32_t>();

    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");
    qf::throw_if_not<std::runtime_error>(version == QFLV_VERSION, "unsupported file version");
    qf::throw_if_not<std::runtime_error>(lumpcnt > 1, "no lumps present");

    level::purge();

    std::unordered_set<std::uint32_t> loaded_lumps;

    for(std::uint32_t i = 0; i < lumpcnt; ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        auto lumptype = buffer.read<std::uint32_t>();

        qf::throw_if_fmt<std::runtime_error>(loaded_lumps.contains(lumptype), "lump {} present twice", lumptype);

        switch(lumptype) {
            case LUMP_GEO:
                read_geometry(buffer);
                break;

            case LUMP_PVS:
                read_pvs(buffer);
                break;

            case LUMP_ENT:
                read_entities(buffer);
                break;

            case LUMP_RAD:
                read_lightmaps(buffer);
                break;

            case LUMP_IDX:
                read_indices(buffer);
                break;

            case LUMP_VTX:
                read_vertices(buffer);
                break;

            default:
                throw qf::runtime_error("unknown lump type: {}", lumptype);
        }

        loaded_lumps.insert(lumptype);
    }

    auto inv_magic_0 = buffer.read<std::uint8_t>();
    auto inv_magic_1 = buffer.read<std::uint8_t>();
    auto inv_magic_2 = buffer.read<std::uint8_t>();
    auto inv_magic_3 = buffer.read<std::uint8_t>();

    auto inv_signature_valid = true;
    inv_signature_valid = inv_signature_valid && inv_magic_0 == MAGIC_BYTE_3;
    inv_signature_valid = inv_signature_valid && inv_magic_1 == MAGIC_BYTE_2;
    inv_signature_valid = inv_signature_valid && inv_magic_2 == MAGIC_BYTE_1;
    inv_signature_valid = inv_signature_valid && inv_magic_3 == MAGIC_BYTE_0;

    qf::throw_if_not<std::runtime_error>(inv_signature_valid, "invalid file signature");

    if(!buffer.is_ended()) {
        LOG_WARNING("{}: garbage data after expected end-of-file", path_unfucked);
    }
}

void level::save(std::string_view path)
{
    WriteBuffer buffer;

    buffer.write<std::uint8_t>(MAGIC_BYTE_0);
    buffer.write<std::uint8_t>(MAGIC_BYTE_1);
    buffer.write<std::uint8_t>(MAGIC_BYTE_2);
    buffer.write<std::uint8_t>(MAGIC_BYTE_3);

    std::uint32_t lumpcnt = 2; // at least geometry and entities

    if(level::PVS.size()) {
        lumpcnt += 1; // LUMP_PVS
    }

    if(level::indices.size()) {
        lumpcnt += 1; // LUMP_IDX
    }

    if(level::vertices.size()) {
        lumpcnt += 1; // LUMP_VTX
    }

    buffer.write<std::uint32_t>(QFLV_VERSION);
    buffer.write<std::uint32_t>(lumpcnt);

    buffer.write<std::uint32_t>(LUMP_GEO);
    write_geometry(buffer);

    if(level::PVS.size()) {
        buffer.write<std::uint32_t>(LUMP_PVS);
        write_pvs(buffer);
    }

    buffer.write<std::uint32_t>(LUMP_ENT);
    write_entities(buffer);

    if(level::indices.size()) {
        buffer.write<std::uint32_t>(LUMP_IDX);
        write_indices(buffer);
    }

    if(level::vertices.size()) {
        buffer.write<std::uint32_t>(LUMP_VTX);
        write_vertices(buffer);
    }

    buffer.write<std::uint8_t>(MAGIC_BYTE_3);
    buffer.write<std::uint8_t>(MAGIC_BYTE_2);
    buffer.write<std::uint8_t>(MAGIC_BYTE_1);
    buffer.write<std::uint8_t>(MAGIC_BYTE_0);

    auto file = buffer.to_file(path);
    qf::throw_if_not<std::runtime_error>(file, utils::physfs_error());

    PHYSFS_close(file);
}

bool level::safe_load(std::string_view path)
{
    try {
        level::load(path);
        return true;
    }
    catch(const std::exception& ex) {
        LOG_WARNING("{}: save failed: {}", path, ex.what());
        return false;
    }
    catch(...) {
        LOG_WARNING("{}: save failed", path);
        return false;
    }
}

bool level::safe_save(std::string_view path)
{
    try {
        level::save(path);
        return true;
    }
    catch(const std::exception& ex) {
        LOG_WARNING("{}: load failed: {}", path, ex.what());
        return false;
    }
    catch(...) {
        LOG_WARNING("{}: load failed", path);
        return false;
    }
}
