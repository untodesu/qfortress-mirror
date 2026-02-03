#include "core/pch.hh"

#include "core/entity/current_leaf.hh"

#include "core/entity/transform.hh"
#include "core/level/level.hh"

void CurrentLeaf::fixed_update(Level& level)
{
    auto& registry = level.registry();
    auto view = registry.view<Transform>();

    for(auto [entity, transform] : view.each()) {
        registry.emplace_or_replace<CurrentLeaf>(entity, level.find_leaf_index(transform.position()));
    }
}

CurrentLeaf::CurrentLeaf(int leaf_index) : m_leaf_index(leaf_index)
{
    // empty
}
