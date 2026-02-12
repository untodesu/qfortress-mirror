#ifndef CORE_ENTITY_CURRENT_LEAF_HH
#define CORE_ENTITY_CURRENT_LEAF_HH
#pragma once

class Level;

class CurrentLeaf final {
public:
    static void register_component(void);

    static void fixed_update(Level& level);

    explicit CurrentLeaf(std::int32_t leaf_index);

    constexpr operator std::int32_t(void) const noexcept;
    constexpr std::int32_t leaf_index(void) const noexcept;

private:
    std::int32_t m_leaf_index;
};

constexpr CurrentLeaf::operator std::int32_t(void) const noexcept
{
    return m_leaf_index;
}

constexpr std::int32_t CurrentLeaf::leaf_index(void) const noexcept
{
    return m_leaf_index;
}

#endif
