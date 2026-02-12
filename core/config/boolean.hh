#ifndef CORE_CONFIG_BOOLEAN_HH
#define CORE_CONFIG_BOOLEAN_HH
#pragma once

#include "core/config/value.hh"

class ConfigBoolean final : public ConfigValue {
public:
    static bool string_to_bool(std::string_view value);
    static std::string_view bool_to_string(bool value);

    explicit ConfigBoolean(std::string_view name, bool default_value);
    virtual ~ConfigBoolean(void) override = default;

    constexpr operator bool(void) const noexcept;

    constexpr bool boolean(void) const noexcept;
    void set_boolean(bool value) noexcept;

    virtual std::string_view value(void) const noexcept override;
    virtual bool set_value(std::string_view value) noexcept override;

private:
    bool m_boolean;
};

constexpr ConfigBoolean::operator bool(void) const noexcept
{
    return m_boolean;
}

inline constexpr bool ConfigBoolean::boolean(void) const noexcept
{
    return m_boolean;
}

#endif
