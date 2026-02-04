#include "core/pch.hh"

#include "core/entity/current_leaf.hh"

#include "core/entity/components.hh"
#include "core/entity/transform.hh"
#include "core/level/level.hh"

static JSON_Value* serialize_current_leaf(const entt::registry& registry, entt::entity entity)
{
    assert(registry.valid(entity));

    auto& current_leaf = registry.get<CurrentLeaf>(entity);

    return json_value_init_number(current_leaf.leaf_index());
}

static void deserialize_current_leaf(entt::registry& registry, entt::entity entity, const JSON_Value* jsonv)
{
    assert(registry.valid(entity));
    assert(jsonv);

    auto leaf_index = json_value_get_number(jsonv);
    assert(std::isfinite(leaf_index));

    registry.emplace_or_replace<CurrentLeaf>(entity, static_cast<int>(leaf_index));
}

void CurrentLeaf::register_component(void)
{
    components::register_component("current_leaf", &serialize_current_leaf, &deserialize_current_leaf);
}

void CurrentLeaf::fixed_update(void)
{
    auto view = level::registry.view<Transform>();

    for(auto [entity, transform] : view.each()) {
        level::registry.emplace_or_replace<CurrentLeaf>(entity, level::find_leaf_index(transform.position()));
    }
}

CurrentLeaf::CurrentLeaf(int leaf_index) : m_leaf_index(leaf_index)
{
    // empty
}
