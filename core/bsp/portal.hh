#ifndef CORE_BSP_PORTAL_HH
#define CORE_BSP_PORTAL_HH
#pragma once

namespace bsp
{
/// A portal connecting two BSP leaves; contains
/// vertex data for the portal geometry as well as indices
/// of the front and back leaf nodes that the portal connects
struct Portal final {
    std::uint32_t portal_vertex_offset { UINT32_MAX };
    std::uint32_t portal_vertex_count { 0 };
    std::uint32_t front_leaf_index { UINT32_MAX };
    std::uint32_t back_leaf_index { UINT32_MAX };
    std::uint32_t plane_index { UINT32_MAX };
};
} // namespace bsp

#endif
