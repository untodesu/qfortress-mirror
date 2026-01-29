#include "core/pch.hh"

#include "core/config/string.hh"

ConfigString::ConfigString(std::string_view name, std::string_view default_value) : ConfigValue(name), m_string(default_value)
{
    // empty
}

void ConfigString::set_string(std::string_view value)
{
    m_string = value;
}

std::string_view ConfigString::value(void) const noexcept
{
    return m_string;
}

bool ConfigString::set_value(std::string_view value) noexcept
{
    m_string = value;

    return true;
}
