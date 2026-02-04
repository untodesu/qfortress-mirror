#ifndef CORE_LEVEL_HH
#define CORE_LEVEL_HH
#pragma once

class BNode;
struct LevelVertex;

namespace level
{
extern entt::registry registry;
extern std::shared_ptr<BNode> root;
extern std::vector<std::vector<std::uint32_t>> PVS;
extern std::vector<std::uint32_t> indices;
extern std::vector<LevelVertex> vertices;
} // namespace level

namespace level
{
void purge(void);
} // namespace level

namespace level
{
/// Locate a leaf index in which a point is located
/// @param position Position to locate leaf for
/// @return Leaf index or -1 if not found
int find_leaf_index(const Eigen::Vector3f& position);

/// Enumerate BSP nodes visible from a position
/// @param from_leaf Leaf index of the viewer
/// @param position Position of the viewer
/// @param out_nodes Output vector to store visible nodes
void enumerate_visible(int from_leaf, const Eigen::Vector3f& position, std::vector<const BNode*>& out_nodes);

/// Performs a visibility check to see if one leaf can see another
/// @param from_leaf Leaf index of the viewer
/// @param to_leaf Leaf index of the target
/// @return True if visible, false otherwise
bool is_visible(int from_leaf, int to_leaf);
} // namespace level

#endif
