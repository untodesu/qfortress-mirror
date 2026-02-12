#include "core/pch.hh"

#include "core/math/camera.hh"

void math::Camera::set_projection_ortho(float left, float right, float bottom, float top, float z_near, float z_far) noexcept
{
    assert(std::isfinite(left));
    assert(std::isfinite(right));
    assert(std::isfinite(bottom));
    assert(std::isfinite(top));
    assert(std::isfinite(z_near));
    assert(std::isfinite(z_far));
    assert(z_near < z_far);

    m_projection.setIdentity();
    m_projection(0, 0) = 2.0f / (right - left);
    m_projection(1, 1) = 2.0f / (top - bottom);
    m_projection(2, 2) = -2.0f / (z_far - z_near);
    m_projection(0, 3) = -1.0f * (right + left) / (right - left);
    m_projection(1, 3) = -1.0f * (top + bottom) / (top - bottom);
    m_projection(2, 3) = -1.0f * (z_far + z_near) / (z_far - z_near);

    m_dirty = true;
}

void math::Camera::set_projection_perspective(float fov_y, float aspect_ratio, float z_near, float z_far) noexcept
{
    assert(std::isfinite(fov_y));
    assert(std::isfinite(aspect_ratio));
    assert(std::isfinite(z_near));
    assert(std::isfinite(z_far));
    assert(z_far > z_near);

    auto inv_tangent = 1.0f / std::tan(0.5f * fov_y);

    m_projection.setZero();
    m_projection(0, 0) = inv_tangent / aspect_ratio;
    m_projection(1, 1) = inv_tangent;
    m_projection(2, 2) = -1.0f * (z_far + z_near) / (z_far - z_near);
    m_projection(3, 2) = -1.0f;
    m_projection(2, 3) = -2.0f * z_far * z_near / (z_far - z_near);

    m_dirty = true;
}

// Gracefully contributed into Voxelius by PQCraft himself in 2024 and now
// made its way into QFortress, making the two projects related to PlatinumSrc
void math::Camera::set_view(const Eigen::Vector3f& position, const Eigen::Quaternionf& quaternion) noexcept
{
    Eigen::Vector3f forward(quaternion * Eigen::Vector3f::UnitZ());
    Eigen::Vector3f up(quaternion * Eigen::Vector3f::UnitY());

    m_view.setIdentity();
    m_view(0, 0) = forward.y() * up.z() - forward.z() * up.y();
    m_view(0, 1) = forward.z() * up.x() - forward.x() * up.z();
    m_view(0, 2) = forward.x() * up.y() - forward.y() * up.x();
    m_view(0, 3) = -m_view(0, 0) * position.x() - m_view(0, 1) * position.y() - m_view(0, 2) * position.z();
    m_view(1, 0) = up.x();
    m_view(1, 1) = up.y();
    m_view(1, 2) = up.z();
    m_view(1, 3) = -up.x() * position.x() - up.y() * position.y() - up.z() * position.z();
    m_view(2, 0) = -forward.x();
    m_view(2, 1) = -forward.y();
    m_view(2, 2) = -forward.z();
    m_view(2, 3) = forward.x() * position.x() + forward.y() * position.y() + forward.z() * position.z();

    m_dirty = true;
}

void math::Camera::set_view(const Eigen::Vector3f& position, const Eigen::Vector3f& euler_angles) noexcept
{
    Eigen::AngleAxisf x_rot(euler_angles.x(), Eigen::Vector3f::UnitX()); // pitch
    Eigen::AngleAxisf y_rot(euler_angles.y(), Eigen::Vector3f::UnitY()); // yaw
    Eigen::AngleAxisf z_rot(euler_angles.z(), Eigen::Vector3f::UnitZ()); // roll

    set_view(position, Eigen::Quaternionf(x_rot * y_rot * z_rot));
}

void math::Camera::set_look(const Eigen::Vector3f& position, const Eigen::Vector3f& target) noexcept
{
    Eigen::Vector3f forward((target - position).normalized());
    Eigen::Vector3f right(Eigen::Vector3f::UnitY().cross(forward).normalized());
    Eigen::Vector3f up(forward.cross(right).normalized());

    m_view.setIdentity();
    m_view(0, 0) = right.x();
    m_view(0, 1) = right.y();
    m_view(0, 2) = right.z();
    m_view(0, 3) = -right.dot(position);
    m_view(1, 0) = up.x();
    m_view(1, 1) = up.y();
    m_view(1, 2) = up.z();
    m_view(1, 3) = -up.dot(position);
    m_view(2, 0) = -forward.x();
    m_view(2, 1) = -forward.y();
    m_view(2, 2) = -forward.z();
    m_view(2, 3) = forward.dot(position);

    m_dirty = true;
}

void math::Camera::update(void) noexcept
{
    if(m_dirty) {
        m_view_projection = m_projection * m_view;

        m_dirty = false;
    }
}
