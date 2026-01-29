#include "core/pch.hh"

#include "core/config/value.hh"

ConfigValue::ConfigValue(std::string_view name) : m_name(name)
{
    assert(name.size());
}
