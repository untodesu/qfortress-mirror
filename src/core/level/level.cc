#include "core/pch.hh"

#include "core/level/level.hh"

#include "core/exceptions.hh"
#include "core/level/bnode.hh"
#include "core/level/vertex.hh"

entt::registry level::registry;
std::shared_ptr<BNode> level::root;
std::vector<std::vector<std::uint32_t>> level::PVS;
std::vector<std::uint32_t> level::indices;
std::vector<LevelVertex> level::vertices;

static int find_leaf_index_internal(const BNode* node, const Eigen::Vector3f& position)
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

static void enumerate_visible_internal(const BNode* node, int from_leaf, const Eigen::Vector3f& position,
    std::vector<const BNode*>& out_nodes)
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
            if(level::is_visible(from_leaf, leaf->index)) {
                out_nodes.push_back(node);
            }
        }

        throw qf::logic_error("invalid variant state of BNode::data");
    }
}

void level::purge(void)
{
    level::registry.clear();
    level::root.reset();
    level::PVS.clear();
    level::indices.clear();
    level::vertices.clear();
}

int level::find_leaf_index(const Eigen::Vector3f& position)
{
    return find_leaf_index_internal(level::root.get(), position);
}

void level::enumerate_visible(int from_leaf, const Eigen::Vector3f& position, std::vector<const BNode*>& out_nodes)
{
    enumerate_visible_internal(level::root.get(), from_leaf, position, out_nodes);
}

bool level::is_visible(int from_leaf, int to_leaf)
{
    if(from_leaf < 0 || from_leaf >= level::PVS.size() || to_leaf < 0) {
        return true; // out of bounds, assume visible
    }

    const auto& from_pvs = level::PVS[from_leaf];
    auto block_index = static_cast<std::size_t>(to_leaf / 32);
    auto bit_flag = static_cast<std::uint32_t>(1u << (to_leaf % 32));

    if(block_index < from_pvs.size()) {
        return from_pvs[block_index] & bit_flag;
    }

    return false;
}
