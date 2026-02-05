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

    const BNode* root(void) const noexcept;

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

    /// Generate a flattened list of all the BSP nodes present
    /// @param out_nodes The flattened list of all the nodes
    /// @param out_indices An index mapping for each node
    void flatten_bsp(std::vector<const BNode*>& out_nodes, std::unordered_map<const BNode*, std::int32_t>& out_indices) const;

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
    /// Generate a flattened list of all the BSP nodes present
    /// @param node The node to recurse into
    /// @param out_nodes The flattened list of all the nodes
    /// @param out_indices An index mapping for each node
    void flatten_bsp_internal(const BNode* node, std::vector<const BNode*>& out_nodes,
        std::unordered_map<const BNode*, std::int32_t>& out_indices) const;

    /// Locate a leaf index in which a point is located
    /// @param node The node to recurse into
    /// @param position Position to locate leaf for
    /// @return Leaf index or -1 if not found
    int find_leaf_index_internal(const BNode* node, const Eigen::Vector3f& position) const;

    /// Enumerate BSP nodes visible from a position
    /// @param node The node to recurse into
    /// @param from_leaf Leaf index of the viewer
    /// @param position Position of the viewer
    /// @param out_nodes Output vector to store visible nodes
    void enumerate_visible_internal(const BNode* node, int from_leaf, const Eigen::Vector3f& position,
        std::vector<const BNode*>& out_nodes) const;

    void read_lump_bsp(ReadBuffer& buffer);
    void read_lump_pvs(ReadBuffer& buffer);
    void read_lump_ent(ReadBuffer& buffer);
    void read_lump_rad(ReadBuffer& buffer);
    void read_lump_vtx(ReadBuffer& buffer);

    void write_lump_bsp(WriteBuffer& buffer) const;
    void write_lump_pvs(WriteBuffer& buffer) const;
    void write_lump_ent(WriteBuffer& buffer) const;
    void write_lump_rad(WriteBuffer& buffer) const;
    void write_lump_vtx(WriteBuffer& buffer) const;

    entt::registry m_registry;
    std::shared_ptr<BNode> m_root;
    std::vector<std::vector<std::uint32_t>> m_pvs;
    std::vector<std::uint32_t> m_indices;
    std::vector<LevelVertex> m_vertices;
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

#endif
