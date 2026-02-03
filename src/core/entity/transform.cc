#include "core/pch.hh"

#include "core/entity/transform.hh"

Transform::Transform(const Eigen::Affine3f& affine) noexcept : m_affine(affine)
{
    assert(affine.matrix().allFinite());
}

Transform::Transform(const Eigen::Vector3f& position, const Eigen::Quaternionf& orientation) noexcept
{
    assert(position.allFinite());
    assert(orientation.coeffs().allFinite());

    m_affine.setIdentity();
    m_affine.linear() = orientation.toRotationMatrix();
    m_affine.translation() = position;
}

Transform Transform::moved(const Eigen::Vector3f& shift) const noexcept
{
    Eigen::Affine3f affine(m_affine);
    affine.translation() += shift;
    return Transform(affine);
}

Transform Transform::rotated(const Eigen::Quaternionf& quat_delta) const noexcept
{
    Eigen::Affine3f affine(m_affine);
    affine.linear() = quat_delta.toRotationMatrix() * affine.linear();
    return Transform(affine);
}

Transform Transform::rotated(const Eigen::Vector3f& axis, float angle) const noexcept
{
    Eigen::Affine3f affine(m_affine);
    affine.linear() = Eigen::AngleAxisf(angle, axis.normalized()).toRotationMatrix() * affine.linear();
    return Transform(affine);
}

Transform Transform::rotated(const Eigen::Vector3f& euler_delta) const noexcept
{
    Eigen::Affine3f affine(m_affine);
    affine.linear() = Eigen::AngleAxisf(euler_delta.x(), Eigen::Vector3f::UnitX()).toRotationMatrix() * affine.linear();
    affine.linear() = Eigen::AngleAxisf(euler_delta.y(), Eigen::Vector3f::UnitY()).toRotationMatrix() * affine.linear();
    affine.linear() = Eigen::AngleAxisf(euler_delta.z(), Eigen::Vector3f::UnitZ()).toRotationMatrix() * affine.linear();
    return Transform(affine);
}

Transform Transform::moved_to(const Eigen::Vector3f& position) const noexcept
{
    Eigen::Affine3f affine(m_affine);
    affine.translation() = position;
    return Transform(affine);
}

Transform Transform::rotated_to(const Eigen::Quaternionf& orientation) const noexcept
{
    Eigen::Affine3f affine(m_affine);
    affine.linear() = orientation.toRotationMatrix();
    return Transform(affine);
}

Transform Transform::rotated_to(const Eigen::Vector3f& euler_angles) const noexcept
{
    Eigen::Affine3f affine(m_affine);
    affine.linear() = Eigen::AngleAxisf(euler_angles.x(), Eigen::Vector3f::UnitX()).toRotationMatrix();
    affine.linear() = Eigen::AngleAxisf(euler_angles.y(), Eigen::Vector3f::UnitY()).toRotationMatrix() * affine.linear();
    affine.linear() = Eigen::AngleAxisf(euler_angles.z(), Eigen::Vector3f::UnitZ()).toRotationMatrix() * affine.linear();
    return Transform(affine);
}

Eigen::Vector3f Transform::position(void) const noexcept
{
    return m_affine.translation();
}

Eigen::Vector3f Transform::forward_vector(void) const noexcept
{
    return m_affine.linear().col(2);
}

Eigen::Vector3f Transform::right_vector(void) const noexcept
{
    return m_affine.linear().col(0);
}

Eigen::Vector3f Transform::up_vector(void) const noexcept
{
    return m_affine.linear().col(1);
}
