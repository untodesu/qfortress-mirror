#ifndef CORE_ENTITY_TRANSFORM_HH
#define CORE_ENTITY_TRANSFORM_HH
#pragma once

class Transform final {
public:
    static void register_component(void);

    Transform(void) = default;
    explicit Transform(const Eigen::Affine3f& affine) noexcept;
    explicit Transform(const Eigen::Vector3f& position, const Eigen::Quaternionf& orientation) noexcept;

    constexpr const Eigen::Affine3f& affine(void) const noexcept;

    Transform moved(const Eigen::Vector3f& shift) const noexcept;
    Transform rotated(const Eigen::Quaternionf& quat_delta) const noexcept;
    Transform rotated(const Eigen::Vector3f& axis, float angle) const noexcept;
    Transform rotated(const Eigen::Vector3f& euler_delta) const noexcept;

    Transform moved_to(const Eigen::Vector3f& position) const noexcept;
    Transform rotated_to(const Eigen::Quaternionf& orientation) const noexcept;
    Transform rotated_to(const Eigen::Vector3f& euler_angles) const noexcept;

    Eigen::Vector3f position(void) const noexcept;
    Eigen::Vector3f forward_vector(void) const noexcept;
    Eigen::Vector3f right_vector(void) const noexcept;
    Eigen::Vector3f up_vector(void) const noexcept;

private:
    Eigen::Affine3f m_affine;
};

constexpr const Eigen::Affine3f& Transform::affine(void) const noexcept
{
    return m_affine;
}

#endif
