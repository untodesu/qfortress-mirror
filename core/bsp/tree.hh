#ifndef CORE_BSP_TREE_HH
#define CORE_BSP_TREE_HH
#pragma once

#include "core/bsp/node.hh"

class ReadBuffer;
class WriteBuffer;

namespace bsp
{
/// A quote-unquote "lump" for the BSP tree structure; contains
/// plane and node data; the tree is stored as a flat array of nodes
/// with indices referencing child nodes; the root node is always at index 0
class Tree final {
public:
    /// Traverses the binary tree front-to-back
    /// @param look The point from which we're traversing the tree
    /// @param nodes Output nodes vector, will be cleared before traversal
    void traverse_ftb(const Eigen::Vector3f& look, std::vector<std::size_t>& nodes) const noexcept;

    /// Traverses the binary tree back-to-front
    /// @param look The point from which we're traversing the tree
    /// @param nodes Output nodes vector, will be cleared before traversal
    void traverse_btf(const Eigen::Vector3f& look, std::vector<std::size_t>& nodes) const noexcept;

    /// Locates a leaf node in which the point resides
    /// @param point The point we're trying to classify
    /// @return Leaf node index or SIZE_MAX if tree is empty
    std::size_t locate(const Eigen::Vector3f& point) const noexcept;

    constexpr const std::vector<Eigen::Hyperplane<float, 3>>& planes(void) const noexcept;
    void set_planes(std::vector<Eigen::Hyperplane<float, 3>> planes) noexcept;

    constexpr const std::vector<bsp::Node>& nodes(void) const noexcept;
    void set_nodes(std::vector<bsp::Node> nodes) noexcept;

private:
    /// Recursive private implementation of traverse_ftb()
    /// @param look The point from which we're traversing the tree
    /// @param index Node index to recurse into
    /// @param nodes Output nodes vector
    void traverse_internal_ftb(const Eigen::Vector3f& look, std::size_t index, std::vector<std::size_t>& nodes) const noexcept;

    /// Recursive private implementation of traverse_btf()
    /// @param look The point from which we're traversing the tree
    /// @param index Node index to recurse into
    /// @param nodes Output nodes vector
    void traverse_internal_btf(const Eigen::Vector3f& look, std::size_t index, std::vector<std::size_t>& nodes) const noexcept;

    /// Recursive private implementation of locate()
    /// @param point The point we're trying to classify
    /// @param index Node index to recurse into
    /// @return Leaf node index
    std::size_t locate_internal(const Eigen::Vector3f& point, std::size_t index) const noexcept;

    std::vector<Eigen::Hyperplane<float, 3>> m_planes;
    std::vector<bsp::Node> m_nodes;
};
} // namespace bsp

constexpr const std::vector<Eigen::Hyperplane<float, 3>>& bsp::Tree::planes(void) const noexcept
{
    return m_planes;
}

constexpr const std::vector<bsp::Node>& bsp::Tree::nodes(void) const noexcept
{
    return m_nodes;
}

#endif
