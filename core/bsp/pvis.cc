#include "core/pch.hh"

#include "core/bsp/pvis.hh"

void bsp::PVis::set_portals(std::vector<bsp::Portal> portals) noexcept
{
    m_portals = std::move(portals);
}

void bsp::PVis::set_vertices(std::vector<Eigen::Vector3f> vertices) noexcept
{
    m_vertices = std::move(vertices);
}

void bsp::PVis::set_leaf_portals(std::vector<std::uint32_t> leaf_portals) noexcept
{
    m_leaf_portals = std::move(leaf_portals);
}
