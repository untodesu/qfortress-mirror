#ifndef CORE_BSP_NODE_HH
#define CORE_BSP_NODE_HH
#pragma once

namespace bsp
{
struct Chain final {
    std::uint32_t plane_index { UINT32_MAX };
    std::uint32_t front_index { UINT32_MAX };
    std::uint32_t back_index { UINT32_MAX };
};

struct Leaf final {
    std::uint32_t plane_index { UINT32_MAX };
    std::uint32_t idx_buffer_offset { UINT32_MAX };
    std::uint32_t idx_buffer_count { UINT32_MAX };
    std::uint32_t material_index { UINT32_MAX };
};

using Node = std::variant<Chain, Leaf>;
} // namespace bsp

#endif
