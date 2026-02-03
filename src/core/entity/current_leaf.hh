#ifndef CORE_ENTITY_CURRENT_LEAF_HH
#define CORE_ENTITY_CURRENT_LEAF_HH
#pragma once

class CurrentLeaf final {
public:
    static void fixed_update(void);

    explicit CurrentLeaf(int leaf_index);

    constexpr operator int(void) const noexcept;
    constexpr int leaf_index(void) const noexcept;

private:
    int m_leaf_index;
};

constexpr CurrentLeaf::operator int(void) const noexcept
{
    return m_leaf_index;
}

constexpr int CurrentLeaf::leaf_index(void) const noexcept
{
    return m_leaf_index;
}

#endif
