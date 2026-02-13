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
/// A quote-unquote "lump" for potentially visible set (PVS) data; contains
/// precomputed visibility information for each leaf in the BSP tree; this
/// allows us to quickly determine which parts of the level are potentially
/// visible from a given leaf; more complicated culling is done via bsp::PVis
class PVS final {
public:
    /// Checks if a BSP leaf is potentially visible from an another leaf
    /// @param from_leaf Leaf index from which we're looking
    /// @param target_leaf Leaf index which is to be checked for visibility
    /// @return True when target_leaf is potentially visible from from_leaf
    bool is_visible(std::size_t from_leaf, std::size_t target_leaf) const noexcept;

    /// Gets the set of potentially visible leaves from a given leaf
    /// @param from_leaf Leaf index from which we're looking
    /// @param out_leaves Output set of leaf indices, will be cleared before filling
    /// @return True if from_leaf is valid and out_leaves was filled, false if from_leaf is out of bounds and out_leaves was cleared
    bool get_visible_leaves(std::size_t from_leaf, std::unordered_set<std::uint32_t>& out_leaves) const noexcept;

    /// Gets the vector of potentially visible leaves from a given leaf as a vector
    /// @param from_leaf Leaf index from which we're looking
    /// @param out_leaves Output vector of leaf indices, will be cleared before filling
    /// @return True if from_leaf is valid and out_leaves was filled, false if from_leaf is out of bounds and out_leaves was cleared
    bool get_visible_leaves(std::size_t from_leaf, std::vector<std::uint32_t>& out_leaves) const noexcept;

    constexpr const std::vector<std::unordered_set<std::uint32_t>>& visdata(void) const noexcept;
    void set_visdata(std::vector<std::unordered_set<std::uint32_t>> visdata) noexcept;

private:
    std::vector<std::unordered_set<std::uint32_t>> m_visdata;
};
} // namespace bsp

constexpr const std::vector<std::unordered_set<std::uint32_t>>& bsp::PVS::visdata(void) const noexcept
{
    return m_visdata;
}

#endif
