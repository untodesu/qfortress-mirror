#ifndef CORE_BSP_PVIS_HH
#define CORE_BSP_PVIS_HH
#pragma once

#include "core/bsp/portal.hh"

namespace bsp
{
/// A quote-unquote "lump" for portal visibility (PVis) data; contains
/// a list of portals and their geometry as well as a mapping from leaf
/// nodes to the portals that are connected to them;
class PVis final {
public:
    constexpr const std::vector<bsp::Portal>& portals(void) const noexcept;
    void set_portals(std::vector<bsp::Portal> portals) noexcept;

    constexpr const std::vector<Eigen::Vector3f>& vertices(void) const noexcept;
    void set_vertices(std::vector<Eigen::Vector3f> vertices) noexcept;

    constexpr const std::vector<std::uint32_t>& leaf_portals(void) const noexcept;
    void set_leaf_portals(std::vector<std::uint32_t> leaf_portals) noexcept;

private:
    std::vector<bsp::Portal> m_portals;
    std::vector<Eigen::Vector3f> m_vertices;
    std::vector<std::uint32_t> m_leaf_portals;
};
} // namespace bsp

constexpr const std::vector<bsp::Portal>& bsp::PVis::portals(void) const noexcept
{
    return m_portals;
}

constexpr const std::vector<Eigen::Vector3f>& bsp::PVis::vertices(void) const noexcept
{
    return m_vertices;
}

constexpr const std::vector<std::uint32_t>& bsp::PVis::leaf_portals(void) const noexcept
{
    return m_leaf_portals;
}

#endif
