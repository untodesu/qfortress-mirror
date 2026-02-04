#include "core/pch.hh"

#include "core/entity/transform.hh"

#include "core/entity/components.hh"

static JSON_Value* serialize_transform(const entt::registry& registry, entt::entity entity)
{
    assert(registry.valid(entity));

    if(const auto transform = registry.try_get<Transform>(entity)) {
        const auto& affine = transform->affine();

        auto jsonv = json_value_init_array();
        auto json = json_value_get_array(jsonv);
        assert(json);

        Eigen::Vector3f linear_xplus(affine.linear().col(0));
        json_array_append_number(json, linear_xplus.x());
        json_array_append_number(json, linear_xplus.y());
        json_array_append_number(json, linear_xplus.z());

        Eigen::Vector3f linear_yplus(affine.linear().col(1));
        json_array_append_number(json, linear_yplus.x());
        json_array_append_number(json, linear_yplus.y());
        json_array_append_number(json, linear_yplus.z());

        Eigen::Vector3f linear_zplus(affine.linear().col(2));
        json_array_append_number(json, linear_zplus.x());
        json_array_append_number(json, linear_zplus.y());
        json_array_append_number(json, linear_zplus.z());

        Eigen::Vector3f translation(affine.translation());
        json_array_append_number(json, translation.x());
        json_array_append_number(json, translation.y());
        json_array_append_number(json, translation.z());

        assert(12 == json_array_get_count(json));

        return jsonv;
    }

    return nullptr;
}

static void deserialize_transform(entt::registry& registry, entt::entity entity, const JSON_Value* jsonv)
{
    assert(registry.valid(entity));
    assert(jsonv);

    const auto json = json_value_get_array(jsonv);
    assert(json);

    assert(12 == json_array_get_count(json));

    Eigen::Vector3f linear_xplus;
    linear_xplus.x() = static_cast<float>(json_array_get_number(json, 0));
    linear_xplus.y() = static_cast<float>(json_array_get_number(json, 1));
    linear_xplus.z() = static_cast<float>(json_array_get_number(json, 2));
    assert(linear_xplus.allFinite());

    Eigen::Vector3f linear_yplus;
    linear_yplus.x() = static_cast<float>(json_array_get_number(json, 3));
    linear_yplus.y() = static_cast<float>(json_array_get_number(json, 4));
    linear_yplus.z() = static_cast<float>(json_array_get_number(json, 5));
    assert(linear_yplus.allFinite());

    Eigen::Vector3f linear_zplus;
    linear_zplus.x() = static_cast<float>(json_array_get_number(json, 6));
    linear_zplus.y() = static_cast<float>(json_array_get_number(json, 7));
    linear_zplus.z() = static_cast<float>(json_array_get_number(json, 8));
    assert(linear_zplus.allFinite());

    Eigen::Vector3f translation;
    translation.x() = static_cast<float>(json_array_get_number(json, 9));
    translation.y() = static_cast<float>(json_array_get_number(json, 10));
    translation.z() = static_cast<float>(json_array_get_number(json, 11));
    assert(translation.allFinite());

    Eigen::Affine3f affine(Eigen::Affine3f::Identity());
    affine.linear().col(0) = linear_xplus;
    affine.linear().col(1) = linear_yplus;
    affine.linear().col(2) = linear_zplus;
    affine.translation() = translation;

    assert(affine.matrix().allFinite());

    registry.emplace_or_replace<Transform>(entity, affine);
}

void Transform::register_component(void)
{
    components::register_component("transform", &serialize_transform, &deserialize_transform);
}

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
