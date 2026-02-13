#include "core/pch.hh"

#include "core/bsp/pvs.hh"

#include "core/bsp/tree.hh"

bool bsp::PVS::is_visible(std::size_t from_leaf, std::size_t target_leaf) const noexcept
{
    if(from_leaf >= m_visdata.size() || target_leaf == SIZE_MAX) {
        return true; // out of bounds, assume visible
    }

    return m_visdata[from_leaf].contains(static_cast<std::uint32_t>(target_leaf));
}

bool bsp::PVS::get_visible_leaves(std::size_t from_leaf, std::unordered_set<std::uint32_t>& out_leaves) const noexcept
{
    out_leaves.clear();

    if(from_leaf >= m_visdata.size()) {
        return false; // out of bounds
    }

    out_leaves = m_visdata[from_leaf];

    return true;
}

bool bsp::PVS::get_visible_leaves(std::size_t from_leaf, std::vector<std::uint32_t>& out_leaves) const noexcept
{
    out_leaves.clear();

    if(from_leaf >= m_visdata.size()) {
        return false; // out of bounds
    }

    out_leaves.assign(m_visdata[from_leaf].cbegin(), m_visdata[from_leaf].cend());

    return true;
}

void bsp::PVS::set_visdata(std::vector<std::unordered_set<std::uint32_t>> visdata) noexcept
{
    m_visdata = std::move(visdata);
}
