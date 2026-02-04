#ifndef CORE_LEVEL_VERTEX_HH
#define CORE_LEVEL_VERTEX_HH
#pragma once

struct LevelVertex final {
    Eigen::Vector3f position; ///< Vertex position
    Eigen::Vector3f normal;   ///< Vertex normal
    Eigen::Vector4f tangent;  ///< Tangent
    Eigen::Vector2f texcoord; ///< Color texture UV
    Eigen::Vector2f lightmap; ///< Lightmap UV
};

#endif
