#ifndef CORE_BSP_NODE_HH
#define CORE_BSP_NODE_HH
#pragma once

namespace bsp
{
/// A chain node in the BSP tree; doesn't have
/// geometry and is an internal node in the tree
struct Chain final {
    std::uint32_t plane_index { UINT32_MAX };
    std::uint32_t front_index { UINT32_MAX };
    std::uint32_t back_index { UINT32_MAX };
};

/// A leaf node in the BSP tree; contains
/// geometry and is a terminal node in the tree;
struct Leaf final {
    std::uint32_t plane_index { UINT32_MAX };
    std::uint32_t idx_buffer_offset { UINT32_MAX };
    std::uint32_t idx_buffer_count { UINT32_MAX };
    std::uint32_t material_index { UINT32_MAX };
    std::uint32_t lightmap_index { UINT32_MAX };
    std::uint32_t portal_offset { UINT32_MAX };
    std::uint32_t portal_count { 0 };
};

using Node = std::variant<Chain, Leaf>;
} // namespace bsp

#endif
