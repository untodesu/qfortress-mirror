#include "core/pch.hh"

#include "core/bsp/tree.hh"

#include "core/buffer.hh"
#include "core/exceptions.hh"

void bsp::Tree::traverse_ftb(const Eigen::Vector3f& look, std::vector<std::size_t>& nodes) const noexcept
{
    assert(look.allFinite());

    nodes.clear();

    traverse_internal_ftb(look, 0, nodes);
}

void bsp::Tree::traverse_btf(const Eigen::Vector3f& look, std::vector<std::size_t>& nodes) const noexcept
{
    assert(look.allFinite());

    nodes.clear();

    traverse_internal_btf(look, 0, nodes);
}

std::size_t bsp::Tree::locate(const Eigen::Vector3f& point) const noexcept
{
    assert(point.allFinite());

    return locate_internal(point, 0);
}

void bsp::Tree::set_planes(std::vector<Eigen::Hyperplane<float, 3>> planes) noexcept
{
    m_planes = std::move(planes);
}

void bsp::Tree::set_nodes(std::vector<bsp::Node> nodes) noexcept
{
    m_nodes = std::move(nodes);
}

void bsp::Tree::traverse_internal_ftb(const Eigen::Vector3f& look, std::size_t index, std::vector<std::size_t>& nodes) const noexcept
{
    if(index < m_nodes.size()) {
        auto& node = m_nodes[index];

        if(auto chain = std::get_if<bsp::Chain>(&node)) {
            auto& hyperplane = m_planes[chain->plane_index];
            auto distance = hyperplane.signedDistance(look);

            if(distance < 0.0f) {
                traverse_internal_ftb(look, chain->back_index, nodes);
                nodes.push_back(index);
                traverse_internal_ftb(look, chain->front_index, nodes);
            }
            else {
                traverse_internal_ftb(look, chain->front_index, nodes);
                nodes.push_back(index);
                traverse_internal_ftb(look, chain->back_index, nodes);
            }
        }
        else {
            nodes.push_back(index);
        }
    }
}

void bsp::Tree::traverse_internal_btf(const Eigen::Vector3f& look, std::size_t index, std::vector<std::size_t>& nodes) const noexcept
{
    if(index < m_nodes.size()) {
        auto& node = m_nodes[index];

        if(auto chain = std::get_if<bsp::Chain>(&node)) {
            auto& hyperplane = m_planes[chain->plane_index];
            auto distance = hyperplane.signedDistance(look);

            if(distance < 0.0f) {
                traverse_internal_btf(look, chain->front_index, nodes);
                nodes.push_back(index);
                traverse_internal_btf(look, chain->back_index, nodes);
            }
            else {
                traverse_internal_btf(look, chain->back_index, nodes);
                nodes.push_back(index);
                traverse_internal_btf(look, chain->front_index, nodes);
            }
        }
        else {
            nodes.push_back(index);
        }
    }
}

std::size_t bsp::Tree::locate_internal(const Eigen::Vector3f& point, std::size_t index) const noexcept
{
    if(index < m_nodes.size()) {
        auto& node = m_nodes[index];

        if(auto chain = std::get_if<bsp::Chain>(&node)) {
            auto& hyperplane = m_planes[chain->plane_index];
            auto distance = hyperplane.signedDistance(point);

            if(distance < 0.0f) {
                return locate_internal(point, chain->back_index);
            }
            else {
                return locate_internal(point, chain->front_index);
            }
        }
        else {
            return index;
        }
    }

    return SIZE_MAX;
}
