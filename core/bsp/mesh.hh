#ifndef CORE_BSP_MESH_HH
#define CORE_BSP_MESH_HH
#pragma once

#include "core/bsp/node.hh"

namespace bsp
{
struct Vertex final {
    Eigen::Vector3f position; ///< Vertex position
    Eigen::Vector3f normal;   ///< Surface normal
    Eigen::Vector2f texcoord; ///< Texture UV coordinates
    Eigen::Vector2f lightmap; ///< Lightmap UV coordinates
};
} // namespace bsp

namespace bsp
{
/// A quote-unquote "lump" for level geometry; contains
/// vertex and index data that is ready to be uploaded to
/// the GPU as-is without any further processing; index buffer
/// data is referenced by bsp::Leaf idx_buffer_offset and idx_buffer_count fields
class Mesh final {
public:
    constexpr const std::vector<bsp::Vertex>& vertices(void) const noexcept;
    void set_vertices(std::vector<bsp::Vertex> vertices) noexcept;

    constexpr const std::vector<std::uint32_t>& indices(void) const noexcept;
    void set_indices(std::vector<std::uint32_t> indices) noexcept;

private:
    std::vector<bsp::Vertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
};
} // namespace bsp

constexpr const std::vector<bsp::Vertex>& bsp::Mesh::vertices(void) const noexcept
{
    return m_vertices;
}

constexpr const std::vector<std::uint32_t>& bsp::Mesh::indices(void) const noexcept
{
    return m_indices;
}

#endif
