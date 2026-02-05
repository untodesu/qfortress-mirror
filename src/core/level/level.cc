#include "core/pch.hh"

#include "core/level/level.hh"

#include "core/buffer.hh"
#include "core/entity/components.hh"
#include "core/exceptions.hh"
#include "core/level/bnode.hh"
#include "core/level/vertex.hh"
#include "core/utils/physfs.hh"
#include "core/utils/string.hh"

constexpr static std::uint8_t MAGIC_BYTE_0 = 'Q';
constexpr static std::uint8_t MAGIC_BYTE_1 = 'F';
constexpr static std::uint8_t MAGIC_BYTE_2 = 'L';
constexpr static std::uint8_t MAGIC_BYTE_3 = 'V';

constexpr static std::uint32_t QFLV_VERSION = 1;

constexpr static std::uint32_t LUMP_BSP = 1; ///< Geometry nodes
constexpr static std::uint32_t LUMP_PVS = 2; ///< Potentially visible set
constexpr static std::uint32_t LUMP_MAT = 3; ///< Materials string table
constexpr static std::uint32_t LUMP_ENT = 4; ///< Entity data as a JSON string
constexpr static std::uint32_t LUMP_RAD = 5; ///< Lightmaps
constexpr static std::uint32_t LUMP_VTX = 6; ///< Vertex and index buffer

struct ProtoBNode final {
    float plane_coefs[4];
    std::int32_t leaf_index { -1 };
    std::int32_t front { -1 };
    std::int32_t back { -1 };
    std::int32_t material { -1 };
    std::int32_t ebo_offset { -1 };
    std::int32_t ebo_count { -1 };
};

void Level::set_geometry(std::vector<std::uint32_t> new_indices, std::vector<LevelVertex> new_vertices) noexcept
{
    m_indices = std::move(new_indices);
    m_vertices = std::move(new_vertices);
}

void Level::set_nodes(std::vector<BNode> new_nodes, std::int32_t new_root) noexcept
{
    m_nodes = std::move(new_nodes);
    m_root_node = new_root;
}

void Level::set_materials(std::vector<std::string> new_materials) noexcept
{
    m_materials = std::move(new_materials);
}

void Level::purge(void) noexcept
{
    m_registry.clear();

    m_nodes.clear();
    m_materials.clear();
    m_pvs.clear();
    m_indices.clear();
    m_vertices.clear();

    m_root_node = -1;
}

void Level::load(std::string_view path)
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
    qf::throw_if_not<std::runtime_error>(lumpcnt > 0, "no lumps present");

    purge();

    std::unordered_set<std::uint32_t> loaded_lumps;

    for(std::uint32_t i = 0; i < lumpcnt; ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        auto lumptype = buffer.read<std::uint32_t>();

        qf::throw_if_fmt<std::runtime_error>(loaded_lumps.contains(lumptype), "lump {} present twice", lumptype);

        switch(lumptype) {
            case LUMP_BSP:
                read_lump_bsp(buffer);
                break;

            case LUMP_PVS:
                read_lump_pvs(buffer);
                break;

            case LUMP_ENT:
                read_lump_ent(buffer);
                break;

            case LUMP_RAD:
                read_lump_rad(buffer);
                break;

            case LUMP_VTX:
                read_lump_vtx(buffer);
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

void Level::save(std::string_view path) const
{
    WriteBuffer buffer;

    buffer.write<std::uint8_t>(MAGIC_BYTE_0);
    buffer.write<std::uint8_t>(MAGIC_BYTE_1);
    buffer.write<std::uint8_t>(MAGIC_BYTE_2);
    buffer.write<std::uint8_t>(MAGIC_BYTE_3);

    std::uint32_t lumpcnt = 0;

    if(m_nodes) {
        lumpcnt += 1; // LUMP_BSP
    }

    if(m_pvs.size()) {
        lumpcnt += 1; // LUMP_PVS
    }

    if(m_registry.view<entt::entity>().size()) {
        lumpcnt += 1; // LUMP_ENT
    }

    if(m_vertices.size() && m_indices.size()) {
        lumpcnt += 1; // LUMP_VTX
    }

    buffer.write<std::uint32_t>(QFLV_VERSION);
    buffer.write<std::uint32_t>(lumpcnt);

    if(m_root) {
        buffer.write<std::uint32_t>(LUMP_BSP);
        write_lump_bsp(buffer);
    }

    if(m_pvs.size()) {
        buffer.write<std::uint32_t>(LUMP_PVS);
        write_lump_pvs(buffer);
    }

    if(m_registry.view<entt::entity>().size()) {
        buffer.write<std::uint32_t>(LUMP_ENT);
        write_lump_ent(buffer);
    }

    if(m_vertices.size() && m_indices.size()) {
        buffer.write<std::uint32_t>(LUMP_VTX);
        write_lump_vtx(buffer);
    }

    buffer.write<std::uint8_t>(MAGIC_BYTE_3);
    buffer.write<std::uint8_t>(MAGIC_BYTE_2);
    buffer.write<std::uint8_t>(MAGIC_BYTE_1);
    buffer.write<std::uint8_t>(MAGIC_BYTE_0);

    auto file = buffer.to_file(path);
    qf::throw_if_not<std::runtime_error>(file, utils::physfs_error());

    PHYSFS_close(file);
}

bool Level::load_safe(std::string_view path) noexcept
{
    try {
        load(path);
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

bool Level::save_safe(std::string_view path) const noexcept
{
    try {
        save(path);
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

void Level::flatten_bsp(std::vector<const BNode*>& out_nodes, std::unordered_map<const BNode*, std::int32_t>& out_indices) const
{
    out_nodes.clear();
    out_indices.clear();

    flatten_bsp_internal(m_root.get(), out_nodes, out_indices);
}

int Level::find_leaf_index(const Eigen::Vector3f& position) const
{
    return find_leaf_index_internal(m_root.get(), position);
}

void Level::enumerate_visible(int from_leaf, const Eigen::Vector3f& position, std::vector<const BNode*>& out_nodes) const
{
    out_nodes.clear();

    enumerate_visible_internal(m_root.get(), from_leaf, position, out_nodes);
}

bool Level::is_visible(int from_leaf, int to_leaf) const
{
    if(from_leaf < 0 || from_leaf >= m_pvs.size() || to_leaf < 0) {
        return true; // out of bounds, assume visible
    }

    const auto& from_pvs = m_pvs[from_leaf];
    auto block_index = static_cast<std::size_t>(to_leaf / 32);
    auto bit_flag = static_cast<std::uint32_t>(1u << (to_leaf % 32));

    if(block_index < from_pvs.size()) {
        return from_pvs[block_index] & bit_flag;
    }

    return false;
}

void Level::flatten_bsp_internal(const BNode* node, std::vector<const BNode*>& out_nodes,
    std::unordered_map<const BNode*, std::int32_t>& out_indices) const
{
    if(node) {
        out_indices.insert_or_assign(node, static_cast<std::int32_t>(out_nodes.size()));
        out_nodes.push_back(node);

        if(const auto internal = std::get_if<BNode::Internal>(&node->data)) {
            flatten_bsp_internal(internal->front.get(), out_nodes, out_indices);
            flatten_bsp_internal(internal->back.get(), out_nodes, out_indices);
        }
    }
}

int Level::find_leaf_index_internal(const BNode* node, const Eigen::Vector3f& position) const
{
    assert(position.allFinite());

    if(node) {
        if(const auto leaf = std::get_if<BNode::Leaf>(&node->data)) {
            return leaf->index;
        }

        if(const auto internal = std::get_if<BNode::Internal>(&node->data)) {
            auto distance = internal->plane.signedDistance(position);

            if(distance >= 0.0f) {
                return find_leaf_index_internal(internal->front.get(), position);
            }
            else {
                return find_leaf_index_internal(internal->back.get(), position);
            }
        }

        throw qf::logic_error("invalid variant state of BNode::data");
    }

    return -1;
}

void Level::enumerate_visible_internal(const BNode* node, int from_leaf, const Eigen::Vector3f& position,
    std::vector<const BNode*>& out_nodes) const
{
    assert(position.allFinite());

    if(node) {
        if(const auto internal = std::get_if<BNode::Internal>(&node->data)) {
            auto distance = internal->plane.signedDistance(position);

            if(distance >= 0.0f) {
                enumerate_visible_internal(internal->back.get(), from_leaf, position, out_nodes);
                out_nodes.push_back(node);
                enumerate_visible_internal(internal->front.get(), from_leaf, position, out_nodes);
            }
            else {
                enumerate_visible_internal(internal->front.get(), from_leaf, position, out_nodes);
                out_nodes.push_back(node);
                enumerate_visible_internal(internal->back.get(), from_leaf, position, out_nodes);
            }
        }
        else if(const auto leaf = std::get_if<BNode::Leaf>(&node->data)) {
            if(is_visible(from_leaf, leaf->index)) {
                out_nodes.push_back(node);
            }
        }

        throw qf::logic_error("invalid variant state of BNode::data");
    }
}

void Level::read_lump_bsp(ReadBuffer& buffer)
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

    m_root = nodes[0];
}

void Level::read_lump_pvs(ReadBuffer& buffer)
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

        m_pvs.push_back(pvsentry);
    }

    qf::throw_if_not<std::runtime_error>(nodecnt == m_pvs.size(), "PVS size mismatch");
}

void Level::read_lump_mat(ReadBuffer& buffer)
{
}

void Level::read_lump_ent(ReadBuffer& buffer)
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
        auto entity = m_registry.create(entity_request);
        qf::throw_if_not<std::runtime_error>(entity_request == entity, "entity id mismatch");

        components::deserialize_entity(m_registry, entity, value);
    }

    json_value_free(jsonv);
}

void Level::read_lump_rad(ReadBuffer& buffer)
{
    // empty
}

void Level::read_lump_vtx(ReadBuffer& buffer)
{
    m_indices.resize(buffer.read<std::uint32_t>());

    for(std::size_t i = 0; i < m_indices.size(); ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        m_indices[i] = buffer.read<std::uint32_t>();
    }

    m_vertices.resize(buffer.read<std::uint32_t>());

    for(std::size_t i = 0; i < m_vertices.size(); ++i) {
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

void Level::write_lump_bsp(WriteBuffer& buffer) const
{
    std::vector<const BNode*> nodes;
    std::unordered_map<const BNode*, std::int32_t> indices;
    flatten_bsp(nodes, indices);

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

void Level::write_lump_pvs(WriteBuffer& buffer) const
{
    auto nodecnt = static_cast<std::uint32_t>(m_pvs.size());

    buffer.write<std::uint32_t>(nodecnt);

    for(const auto& row : m_pvs) {
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

void Level::write_lump_mat(WriteBuffer& buffer) const
{
}

void Level::write_lump_ent(WriteBuffer& buffer) const
{
    auto jsonv = json_value_init_object();
    auto json = json_value_get_object(jsonv);
    assert(jsonv);

    auto view = m_registry.view<entt::entity>();

    for(auto [entity] : view.each()) {
        auto value = components::serialize_entity(m_registry, entity);
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

void Level::write_lump_rad(WriteBuffer& buffer) const
{
    // empty
}

void Level::write_lump_vtx(WriteBuffer& buffer) const
{
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(m_indices.size()));

    for(auto index : m_indices) {
        buffer.write<std::uint32_t>(index);
    }

    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(m_vertices.size()));

    for(const auto& vertex : m_vertices) {
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
