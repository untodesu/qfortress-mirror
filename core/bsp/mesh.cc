#include "core/pch.hh"

#include "core/bsp/mesh.hh"

void bsp::Mesh::set_vertices(std::vector<bsp::Vertex> vertices) noexcept
{
    m_vertices = std::move(vertices);
}

void bsp::Mesh::set_indices(std::vector<std::uint32_t> indices) noexcept
{
    m_indices = std::move(indices);
}
