#include "core/pch.hh"

#include "core/entity/current_leaf.hh"

#include "core/components.hh"
#include "core/entity/transform.hh"
#include "core/level/level.hh"

static JSON_Value* serialize_current_leaf(const entt::registry& registry, entt::entity entity)
{
    assert(registry.valid(entity));

    if(const auto current_leaf = registry.try_get<CurrentLeaf>(entity)) {
        return json_value_init_number(current_leaf->leaf_index());
    }

    return nullptr;
}

static void deserialize_current_leaf(entt::registry& registry, entt::entity entity, const JSON_Value* jsonv)
{
    assert(registry.valid(entity));
    assert(jsonv);

    auto leaf_index = json_value_get_number(jsonv);
    assert(std::isfinite(leaf_index));

    registry.emplace_or_replace<CurrentLeaf>(entity, static_cast<std::int32_t>(leaf_index));
}

void CurrentLeaf::register_component(void)
{
    components::register_component("current_leaf", &serialize_current_leaf, &deserialize_current_leaf);
}

void CurrentLeaf::fixed_update(Level& level)
{
    auto& registry = level.registry();

    auto view = registry.view<Transform>();

    for(auto [entity, transform] : view.each()) {
        registry.emplace_or_replace<CurrentLeaf>(entity, level.find_leaf_index(transform.position()));
    }
}

CurrentLeaf::CurrentLeaf(std::int32_t leaf_index) : m_leaf_index(leaf_index)
{
    // empty
}
