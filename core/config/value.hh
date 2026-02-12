#ifndef CORE_CONFIG_VALUE_HH
#define CORE_CONFIG_VALUE_HH
#pragma once

class ConfigValue {
public:
    explicit ConfigValue(std::string_view name);
    virtual ~ConfigValue(void) = default;

    constexpr std::string_view name(void) const noexcept;

    virtual std::string_view value(void) const noexcept = 0;
    virtual bool set_value(std::string_view new_value) noexcept = 0;

private:
    std::string m_name;
};

constexpr std::string_view ConfigValue::name(void) const noexcept
{
    return m_name;
}

#endif
