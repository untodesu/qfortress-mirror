#include "core/pch.hh"

#include "core/level/level.hh"

#include "core/buffer.hh"
#include "core/entity/components.hh"
#include "core/exceptions.hh"
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

void Level::set_geometry(std::vector<std::uint32_t> new_indices, std::vector<LevelVertex> new_vertices) noexcept
{
    m_indices = std::move(new_indices);
    m_vertices = std::move(new_vertices);
}

void Level::set_nodes(std::vector<Node> new_nodes, std::int32_t new_root) noexcept
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

            case LUMP_MAT:
                read_lump_mat(buffer);
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

    if(m_nodes.size()) {
        lumpcnt += 1; // LUMP_BSP
    }

    if(m_pvs.size()) {
        lumpcnt += 1; // LUMP_PVS
    }

    if(m_materials.size()) {
        lumpcnt += 1; // LUMP_MAT
    }

    if(m_registry.view<entt::entity>().size()) {
        lumpcnt += 1; // LUMP_ENT
    }

    if(m_vertices.size() && m_indices.size()) {
        lumpcnt += 1; // LUMP_VTX
    }

    buffer.write<std::uint32_t>(QFLV_VERSION);
    buffer.write<std::uint32_t>(lumpcnt);

    if(m_nodes.size()) {
        buffer.write<std::uint32_t>(LUMP_BSP);
        write_lump_bsp(buffer);
    }

    if(m_pvs.size()) {
        buffer.write<std::uint32_t>(LUMP_PVS);
        write_lump_pvs(buffer);
    }

    if(m_materials.size()) {
        buffer.write<std::uint32_t>(LUMP_MAT);
        write_lump_mat(buffer);
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

std::int32_t Level::find_leaf_index(const Eigen::Vector3f& position) const
{
    return find_leaf_index_internal(m_root_node, position);
}

void Level::enumerate_visible(std::int32_t from_leaf, const Eigen::Vector3f& position, std::vector<const Node*>& out_nodes) const
{
    out_nodes.clear();

    enumerate_visible_internal(m_root_node, from_leaf, position, out_nodes);
}

bool Level::is_visible(std::int32_t from_leaf, std::int32_t to_leaf) const
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

void Level::enumerate(const Eigen::Vector3f& position, std::vector<const Node*>& out_nodes) const
{
    out_nodes.clear();

    enumerate_internal(m_root_node, position, out_nodes);
}

std::int32_t Level::find_leaf_index_internal(std::int32_t node_index, const Eigen::Vector3f& position) const
{
    assert(position.allFinite());

    if(node_index >= 0 && node_index < m_nodes.size()) {
        const auto node = &m_nodes[node_index];

        if(const auto leaf = std::get_if<Leaf>(node)) {
            return node_index;
        }

        if(const auto internal = std::get_if<Internal>(node)) {
            auto distance = internal->plane.signedDistance(position);

            if(distance >= 0.0f) {
                return find_leaf_index_internal(internal->front, position);
            }
            else {
                return find_leaf_index_internal(internal->back, position);
            }
        }

        throw qf::logic_error("invalid variant state of Node::data");
    }

    return -1;
}

void Level::enumerate_visible_internal(std::int32_t node_index, std::int32_t from_leaf, const Eigen::Vector3f& position,
    std::vector<const Node*>& out_nodes) const
{
    assert(position.allFinite());

    if(node_index >= 0 && node_index < m_nodes.size()) {
        const auto node = &m_nodes[node_index];

        if(const auto internal = std::get_if<Internal>(node)) {
            auto distance = internal->plane.signedDistance(position);

            if(distance >= 0.0f) {
                enumerate_visible_internal(internal->back, from_leaf, position, out_nodes);
                out_nodes.push_back(node);
                enumerate_visible_internal(internal->front, from_leaf, position, out_nodes);
            }
            else {
                enumerate_visible_internal(internal->front, from_leaf, position, out_nodes);
                out_nodes.push_back(node);
                enumerate_visible_internal(internal->back, from_leaf, position, out_nodes);
            }
        }
        else if(const auto leaf = std::get_if<Leaf>(node)) {
            if(is_visible(from_leaf, node_index)) {
                out_nodes.push_back(node);
            }
        }

        throw qf::logic_error("invalid variant state of Node::data");
    }
}

void Level::enumerate_internal(std::int32_t node_index, const Eigen::Vector3f& position, std::vector<const Node*>& out_nodes) const
{
    assert(position.allFinite());

    if(node_index >= 0 && node_index < m_nodes.size()) {
        const auto node = &m_nodes[node_index];

        if(const auto internal = std::get_if<Internal>(node)) {
            auto distance = internal->plane.signedDistance(position);

            if(distance >= 0.0f) {
                enumerate_internal(internal->back, position, out_nodes);
                out_nodes.push_back(node);
                enumerate_internal(internal->front, position, out_nodes);
            }
            else {
                enumerate_internal(internal->front, position, out_nodes);
                out_nodes.push_back(node);
                enumerate_internal(internal->back, position, out_nodes);
            }
        }
        else {
            out_nodes.push_back(node);
        }
    }
}

void Level::read_lump_bsp(ReadBuffer& buffer)
{
    auto nodecnt = buffer.read<std::uint32_t>();
    auto rootnode = buffer.read<std::int32_t>();

    m_nodes.clear();
    m_nodes.reserve(nodecnt);

    qf::throw_if<std::runtime_error>(nodecnt == 0, "empty geometry lump");
    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

    for(std::uint32_t i = 0; i < nodecnt; ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        auto plane_i = buffer.read<float>();
        auto plane_j = buffer.read<float>();
        auto plane_k = buffer.read<float>();
        auto plane_d = buffer.read<float>();
        auto leaf_index = buffer.read<std::int32_t>();
        auto front_index = buffer.read<std::int32_t>();
        auto back_index = buffer.read<std::int32_t>();
        auto material_index = buffer.read<std::int32_t>();
        auto ebo_offset = buffer.read<std::int32_t>();
        auto ebo_count = buffer.read<std::int32_t>();

        if(leaf_index >= 0) {
            Leaf leaf;
            leaf.material = material_index;
            leaf.ebo_offset = ebo_offset;
            leaf.ebo_count = ebo_count;

            m_nodes.emplace_back(std::move(leaf));
        }
        else {
            Internal internal;
            internal.plane = Eigen::Hyperplane<float, 3>(Eigen::Vector3f(plane_i, plane_j, plane_k), plane_d);
            internal.front = front_index;
            internal.back = back_index;

            m_nodes.emplace_back(std::move(internal));
        }
    }

    qf::throw_if<std::runtime_error>(rootnode < 0 || rootnode >= m_nodes.size(), "invalid root node index");

    for(std::uint32_t i = 0; i < m_nodes.size(); ++i) {
        const auto node = &m_nodes[i];

        if(const auto internal = std::get_if<Internal>(node)) {
            qf::throw_if<std::runtime_error>(internal->front >= m_nodes.size(), "invalid front node index");
            qf::throw_if<std::runtime_error>(internal->back >= m_nodes.size(), "invalid back node index");
        }
    }
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
    auto materialcnt = buffer.read<std::uint32_t>();

    qf::throw_if<std::runtime_error>(materialcnt == 0, "empty material lump");
    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

    m_materials.clear();
    m_materials.reserve(materialcnt);

    for(std::uint32_t i = 0; i < materialcnt; ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        m_materials.emplace_back(buffer.read<std::string>());
    }
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
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(m_nodes.size()));

    for(std::int32_t i = 0; i < m_nodes.size(); ++i) {
        const auto node = &m_nodes[i];

        if(const auto leaf = std::get_if<Leaf>(node)) {
            buffer.write<float>(std::numeric_limits<float>::quiet_NaN());
            buffer.write<float>(std::numeric_limits<float>::quiet_NaN());
            buffer.write<float>(std::numeric_limits<float>::quiet_NaN());
            buffer.write<float>(std::numeric_limits<float>::quiet_NaN());

            buffer.write<std::int32_t>(i);
            buffer.write<std::int32_t>(-1);
            buffer.write<std::int32_t>(-1);

            buffer.write<std::int32_t>(leaf->material);
            buffer.write<std::int32_t>(leaf->ebo_offset);
            buffer.write<std::int32_t>(leaf->ebo_count);
        }
        else if(const auto internal = std::get_if<Internal>(node)) {
            buffer.write<float>(internal->plane.coeffs()[0]);
            buffer.write<float>(internal->plane.coeffs()[1]);
            buffer.write<float>(internal->plane.coeffs()[2]);
            buffer.write<float>(internal->plane.coeffs()[3]);

            buffer.write<std::int32_t>(-1);
            buffer.write<std::int32_t>(internal->front);
            buffer.write<std::int32_t>(internal->back);

            buffer.write<std::int32_t>(-1);
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
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(m_materials.size()));

    for(const auto& material : m_materials) {
        buffer.write<std::string_view>(material);
    }
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
