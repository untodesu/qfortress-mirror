#include "core/pch.hh"

#include "core/entity/components.hh"

#include "core/level/level.hh"

struct ComponentInfo final {
    std::string component_name;
    components::serialize_fn serialize_fn;
    components::deserialize_fn deserialize_func;
};

static std::vector<ComponentInfo> s_registered_components;

static const ComponentInfo* find_component_info(std::string_view name)
{
    for(const auto& info : s_registered_components) {
        if(info.component_name == name) {
            return &info;
        }
    }

    return nullptr;
}

void components::register_component(std::string_view name, serialize_fn serializer, deserialize_fn deserializer)
{
    assert(nullptr == find_component_info(name));

    ComponentInfo info;
    info.component_name = name;
    info.serialize_fn = std::move(serializer);
    info.deserialize_func = std::move(deserializer);

    s_registered_components.emplace_back(std::move(info));
}

JSON_Value* components::serialize_entity(entt::entity entity)
{
    assert(level::registry.valid(entity));

    auto jsonv = json_value_init_object();
    auto json = json_value_get_object(jsonv);
    assert(json);

    for(const auto& info : s_registered_components) {
        if(info.serialize_fn) {
            auto componentv = info.serialize_fn(entity);
            assert(componentv);

            json_object_set_value(json, info.component_name.c_str(), componentv);
        }
    }

    return jsonv;
}

void components::deserialize_entity(entt::entity entity, const JSON_Value* jsonv)
{
    assert(level::registry.valid(entity));
    assert(jsonv);

    const auto json = json_value_get_object(jsonv);
    const auto json_count = json_object_get_count(json);

    for(std::size_t i = 0; i < json_count; ++i) {
        const auto component_name = json_object_get_name(json, i);
        const auto componentv = json_object_get_value_at(json, i);

        assert(component_name);
        assert(componentv);

        auto info = find_component_info(component_name);

        if(info == nullptr) {
            LOG_WARNING("unknown component: {}", component_name);

            continue;
        }

        if(!info->deserialize_func) {
            LOG_WARNING("non-deserializable component: {}", component_name);

            continue;
        }

        info->deserialize_func(entity, componentv);
    }
}
