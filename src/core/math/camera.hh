#ifndef CORE_MATH_CAMERA_HH
#define CORE_MATH_CAMERA_HH
#pragma once

namespace math
{
class Camera final {
public:
    constexpr const Eigen::Matrix4f& view_projection(void) const noexcept;
    constexpr const Eigen::Matrix4f& projection(void) const noexcept;
    constexpr const Eigen::Matrix4f& view(void) const noexcept;

    void set_projection_ortho(float left, float right, float bottom, float top, float z_near, float z_far) noexcept;
    void set_projection_perspective(float fov_y, float aspect_ratio, float z_near, float z_far) noexcept;
    void set_view(const Eigen::Vector3f& position, const Eigen::Quaternionf& quaternion) noexcept;
    void set_view(const Eigen::Vector3f& position, const Eigen::Vector3f& euler_angles) noexcept;
    void set_look(const Eigen::Vector3f& position, const Eigen::Vector3f& target) noexcept;

    void update(void) noexcept;

private:
    Eigen::Matrix4f m_view_projection { Eigen::Matrix4f::Identity() };
    Eigen::Matrix4f m_projection { Eigen::Matrix4f::Identity() };
    Eigen::Matrix4f m_view { Eigen::Matrix4f::Identity() };
    bool m_dirty { false };
};
} // namespace math

constexpr const Eigen::Matrix4f& math::Camera::view_projection(void) const noexcept
{
    return m_view_projection;
}

constexpr const Eigen::Matrix4f& math::Camera::projection(void) const noexcept
{
    return m_projection;
}

constexpr const Eigen::Matrix4f& math::Camera::view(void) const noexcept
{
    return m_view;
}

#endif
