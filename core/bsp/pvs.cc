#include "core/pch.hh"

#include "core/bsp/pvs.hh"

#include "core/bsp/tree.hh"

bool bsp::PVS::is_visible(std::size_t from_leaf, std::size_t target_leaf) const noexcept
{
    if(from_leaf >= m_bitmap.size() || target_leaf == SIZE_MAX) {
        return true; // out of bounds, assume visible
    }

    auto& from_pvs = m_bitmap[from_leaf];
    auto block_index = static_cast<std::size_t>(target_leaf / 32);
    auto bit_flag = static_cast<std::uint32_t>(1U << (target_leaf % 32));

    if(block_index < from_pvs.size()) {
        return from_pvs[block_index] & bit_flag;
    }

    return false;
}

void bsp::PVS::traverse_ftb(const bsp::Tree& tree, const Eigen::Vector3f& look, std::vector<std::size_t>& nodes) const noexcept
{
    assert(look.allFinite());

    nodes.clear();

    traverse_internal_ftb(tree.nodes(), tree.planes(), look, 0, tree.locate(look), nodes);
}

void bsp::PVS::traverse_btf(const bsp::Tree& tree, const Eigen::Vector3f& look, std::vector<std::size_t>& nodes) const noexcept
{
    assert(look.allFinite());

    nodes.clear();

    traverse_internal_btf(tree.nodes(), tree.planes(), look, 0, tree.locate(look), nodes);
}

void bsp::PVS::set_bitmap(std::vector<std::vector<std::uint32_t>> bitmap) noexcept
{
    m_bitmap = std::move(bitmap);
}

void bsp::PVS::traverse_internal_ftb(const std::vector<bsp::Node>& nodes, const std::vector<Eigen::Hyperplane<float, 3>>& planes,
    const Eigen::Vector3f& look, std::size_t index, std::size_t from_leaf, std::vector<std::size_t>& out_nodes) const noexcept
{
    if(index < nodes.size()) {
        auto& node = nodes[index];

        if(auto chain = std::get_if<bsp::Chain>(&node)) {
            auto& hyperplane = planes[chain->plane_index];
            auto distance = hyperplane.signedDistance(look);

            if(distance < 0.0f) {
                traverse_internal_ftb(nodes, planes, look, chain->back_index, from_leaf, out_nodes);
                out_nodes.push_back(index);
                traverse_internal_ftb(nodes, planes, look, chain->front_index, from_leaf, out_nodes);
            }
            else {
                traverse_internal_ftb(nodes, planes, look, chain->front_index, from_leaf, out_nodes);
                out_nodes.push_back(index);
                traverse_internal_ftb(nodes, planes, look, chain->back_index, from_leaf, out_nodes);
            }
        }
        else if(is_visible(from_leaf, index)) {
            out_nodes.push_back(index);
        }
    }
}

void bsp::PVS::traverse_internal_btf(const std::vector<bsp::Node>& nodes, const std::vector<Eigen::Hyperplane<float, 3>>& planes,
    const Eigen::Vector3f& look, std::size_t index, std::size_t from_leaf, std::vector<std::size_t>& out_nodes) const noexcept
{
    if(index < nodes.size()) {
        auto& node = nodes[index];

        if(auto chain = std::get_if<bsp::Chain>(&node)) {
            auto& hyperplane = planes[chain->plane_index];
            auto distance = hyperplane.signedDistance(look);

            if(distance < 0.0f) {
                traverse_internal_btf(nodes, planes, look, chain->front_index, from_leaf, out_nodes);
                out_nodes.push_back(index);
                traverse_internal_btf(nodes, planes, look, chain->back_index, from_leaf, out_nodes);
            }
            else {
                traverse_internal_btf(nodes, planes, look, chain->back_index, from_leaf, out_nodes);
                out_nodes.push_back(index);
                traverse_internal_btf(nodes, planes, look, chain->front_index, from_leaf, out_nodes);
            }
        }
        else if(is_visible(from_leaf, index)) {
            out_nodes.push_back(index);
        }
    }
}
