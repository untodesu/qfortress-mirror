#ifndef CORE_BSP_PVS_HH
#define CORE_BSP_PVS_HH
#pragma once

#include "core/bsp/node.hh"

namespace bsp
{
class Tree;
} // namespace bsp

namespace bsp
{
class PVS final {
public:
    /// Checks if a BSP leaf is potentially visible from an another leaf
    /// @param from_leaf Leaf index from which we're looking
    /// @param target_leaf Leaf index which is to be checked for visibility
    /// @return True when target_leaf is potentially visible from from_leaf
    bool is_visible(std::size_t from_leaf, std::size_t target_leaf) const noexcept;

    /// Traverse the binary tree front-to-back with respect to PVS
    /// @param tree BSP tree to traverse
    /// @param look The point from which we're traversing the tree
    /// @param nodes Output nodes vector, will be cleared before traversal
    void traverse_ftb(const bsp::Tree& tree, const Eigen::Vector3f& look, std::vector<std::size_t>& nodes) const noexcept;

    /// Traverse the binary tree back-to-front with respect to PVS
    /// @param tree BSP tree to traverse
    /// @param look The point from which we're traversing the tree
    /// @param nodes Output nodes vector, will be cleared before traversal
    void traverse_btf(const bsp::Tree& tree, const Eigen::Vector3f& look, std::vector<std::size_t>& nodes) const noexcept;

    constexpr const std::vector<std::vector<std::uint32_t>>& bitmap(void) const noexcept;
    void set_bitmap(std::vector<std::vector<std::uint32_t>> bitmap) noexcept;

private:
    /// Recursive private implementation of traverse_ftb()
    /// @param nodes BSP nodes list
    /// @param planes BSP planes list
    /// @param look The point from which we're traversing the tree
    /// @param index Node index to recurse into
    /// @param from_leaf Leaf in which look vector is
    /// @param out_nodes Output nodes vector
    void traverse_internal_ftb(const std::vector<bsp::Node>& nodes, const std::vector<Eigen::Hyperplane<float, 3>>& planes,
        const Eigen::Vector3f& look, std::size_t index, std::size_t from_leaf, std::vector<std::size_t>& out_nodes) const noexcept;

    /// Recursive private implementation of traverse_btf()
    /// @param nodes BSP nodes list
    /// @param planes BSP planes list
    /// @param look The point from which we're traversing the tree
    /// @param index Node index to recurse into
    /// @param from_leaf Leaf in which look vector is
    /// @param out_nodes Output nodes vector
    void traverse_internal_btf(const std::vector<bsp::Node>& nodes, const std::vector<Eigen::Hyperplane<float, 3>>& planes,
        const Eigen::Vector3f& look, std::size_t index, std::size_t from_leaf, std::vector<std::size_t>& out_nodes) const noexcept;

    std::vector<std::vector<std::uint32_t>> m_bitmap;
};
} // namespace bsp

constexpr const std::vector<std::vector<std::uint32_t>>& bsp::PVS::bitmap(void) const noexcept
{
    return m_bitmap;
}

#endif
