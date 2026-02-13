#ifndef CORE_BSP_TREE_HH
#define CORE_BSP_TREE_HH
#pragma once

#include "core/bsp/node.hh"

class ReadBuffer;
class WriteBuffer;

namespace bsp
{
class Tree final {
public:
    void deserialize(ReadBuffer& buffer);
    void serialize(WriteBuffer& buffer) const;

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

    constexpr const std::vector<std::string>& materials(void) const noexcept;
    void set_materials(std::vector<std::string> materials) noexcept;

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

    std::vector<std::string> m_materials;
    std::vector<bsp::Node> m_nodes;
};
} // namespace bsp

constexpr const std::vector<std::string>& bsp::Tree::materials(void) const noexcept
{
    return m_materials;
}

constexpr const std::vector<bsp::Node>& bsp::Tree::nodes(void) const noexcept
{
    return m_nodes;
}

#endif
