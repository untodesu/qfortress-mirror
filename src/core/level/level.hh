#ifndef CORE_LEVEL_HH
#define CORE_LEVEL_HH
#pragma once

#include "core/level/vertex.hh"

class ReadBuffer;
class WriteBuffer;

class BNode;

class Level final {
public:
    Level(void) = default;
    Level(const Level& other) = delete;
    Level& operator=(const Level& other) = delete;

    constexpr entt::registry& registry(void) noexcept;
    constexpr const entt::registry& registry(void) const noexcept;

    constexpr const std::vector<std::uint32_t>& indices(void) const noexcept;
    constexpr const std::vector<LevelVertex>& vertices(void) const noexcept;
    void set_geometry(std::vector<std::uint32_t> new_indices, std::vector<LevelVertex> new_vertices) noexcept;

    constexpr std::int32_t root_node(void) const noexcept;
    constexpr const std::vector<BNode>& nodes(void) const noexcept;
    void set_nodes(std::vector<BNode> new_nodes, std::int32_t new_root) noexcept;

    constexpr const std::vector<std::string>& materials(void) const noexcept;
    void set_materials(std::vector<std::string> new_materials) noexcept;

    /// Purges a level
    void purge(void) noexcept;

    /// Load a level from file
    /// @param path Path to the level file
    /// @throws exceptions if anything bad happens
    void load(std::string_view path);

    /// Save a level to file
    /// @param path Path to the level file
    /// @throws exceptions if anything bad happens
    void save(std::string_view path) const;

    /// Load a level from file
    /// @param path Path to the level file
    /// @return True on success, false otherwise
    bool load_safe(std::string_view path) noexcept;

    /// Save a level to file
    /// @param path Path to the level file
    /// @return True on success, false otherwise
    bool save_safe(std::string_view path) const noexcept;

    /// Locate a leaf index in which a point is located
    /// @param position Position to locate leaf for
    /// @return Leaf index or -1 if not found
    int find_leaf_index(const Eigen::Vector3f& position) const;

    /// Enumerate BSP nodes visible from a position
    /// @param from_leaf Leaf index of the viewer
    /// @param position Position of the viewer
    /// @param out_nodes Output vector to store visible nodes
    void enumerate_visible(int from_leaf, const Eigen::Vector3f& position, std::vector<const BNode*>& out_nodes) const;

    /// Performs a visibility check to see if one leaf can see another
    /// @param from_leaf Leaf index of the viewer
    /// @param to_leaf Leaf index of the target
    /// @return True if visible, false otherwise
    bool is_visible(int from_leaf, int to_leaf) const;

private:
    /// Locate a leaf index in which a point is located
    /// @param node_index The node to recurse into
    /// @param position Position to locate leaf for
    /// @return Leaf index or -1 if not found
    int find_leaf_index_internal(std::int32_t node_index, const Eigen::Vector3f& position) const;

    /// Enumerate BSP nodes visible from a position
    /// @param node_index The node to recurse into
    /// @param from_leaf Leaf index of the viewer
    /// @param position Position of the viewer
    /// @param out_nodes Output vector to store visible nodes
    void enumerate_visible_internal(std::int32_t node_index, int from_leaf, const Eigen::Vector3f& position,
        std::vector<const BNode*>& out_nodes) const;

    void read_lump_bsp(ReadBuffer& buffer);
    void read_lump_pvs(ReadBuffer& buffer);
    void read_lump_mat(ReadBuffer& buffer);
    void read_lump_ent(ReadBuffer& buffer);
    void read_lump_rad(ReadBuffer& buffer);
    void read_lump_vtx(ReadBuffer& buffer);

    void write_lump_bsp(WriteBuffer& buffer) const;
    void write_lump_pvs(WriteBuffer& buffer) const;
    void write_lump_mat(WriteBuffer& buffer) const;
    void write_lump_ent(WriteBuffer& buffer) const;
    void write_lump_rad(WriteBuffer& buffer) const;
    void write_lump_vtx(WriteBuffer& buffer) const;

    entt::registry m_registry;

    std::vector<BNode> m_nodes;
    std::vector<std::string> m_materials;
    std::vector<std::vector<std::uint32_t>> m_pvs;
    std::vector<std::uint32_t> m_indices;
    std::vector<LevelVertex> m_vertices;

    std::int32_t m_root_node;
};

constexpr entt::registry& Level::registry(void) noexcept
{
    return m_registry;
}

inline constexpr const entt::registry& Level::registry(void) const noexcept
{
    return m_registry;
}

inline constexpr const std::vector<std::uint32_t>& Level::indices(void) const noexcept
{
    return m_indices;
}

inline constexpr const std::vector<LevelVertex>& Level::vertices(void) const noexcept
{
    return m_vertices;
}

constexpr std::int32_t Level::root_node(void) const noexcept
{
    return m_root_node;
}

constexpr const std::vector<BNode>& Level::nodes(void) const noexcept
{
    return m_nodes;
}

constexpr const std::vector<std::string>& Level::materials(void) const noexcept
{
    return m_materials;
}

#endif
