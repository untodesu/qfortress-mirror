#include "core/pch.hh"

#include "core/config/boolean.hh"

#include "core/config/arithmetic.hh"

bool ConfigBoolean::string_to_bool(std::string_view value)
{
    auto is_true = false;
    is_true = is_true || 0 == value.compare("true");
    is_true = is_true || 0 == value.compare("yes");
    is_true = is_true || 0 == value.compare("on");
    is_true = is_true || 0 == value.compare("1");

    auto is_false = false;
    is_false = is_false || 0 == value.compare("false");
    is_false = is_false || 0 == value.compare("off");
    is_false = is_false || 0 == value.compare("no");
    is_false = is_false || 0 == value.compare("0");

    return is_true && !is_false;
}

std::string_view ConfigBoolean::bool_to_string(bool value)
{
    if(value)
        return std::string_view("true");
    return std::string_view("false");
}

ConfigBoolean::ConfigBoolean(std::string_view name, bool default_value) : ConfigValue(name), m_boolean(default_value)
{
    // empty
}

void ConfigBoolean::set_boolean(bool value) noexcept
{
    m_boolean = value;
}

std::string_view ConfigBoolean::value(void) const noexcept
{
    return bool_to_string(m_boolean);
}

bool ConfigBoolean::set_value(std::string_view value) noexcept
{
    m_boolean = string_to_bool(value);

    return true;
}
