#ifndef CORE_CONFIG_STRING_HH
#define CORE_CONFIG_STRING_HH
#pragma once

#include "core/config/value.hh"

class ConfigString final : public ConfigValue {
public:
    explicit ConfigString(std::string_view name, std::string_view default_value);
    virtual ~ConfigString(void) override = default;

    constexpr operator std::string_view(void) const;
    constexpr operator const char*(void) const;

    constexpr std::string_view view(void) const;
    constexpr const std::string& string(void) const;
    void set_string(std::string_view value);

    virtual std::string_view value(void) const noexcept override;
    virtual bool set_value(std::string_view value) noexcept override;

private:
    std::string m_string;
};

constexpr ConfigString::operator std::string_view(void) const
{
    return m_string;
}

inline constexpr ConfigString::operator const char*(void) const
{
    return m_string.c_str();
}

inline constexpr std::string_view ConfigString::view(void) const
{
    return m_string;
}

inline constexpr const std::string& ConfigString::string(void) const
{
    return m_string;
}

#endif
