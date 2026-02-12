#ifndef CLIENT_CONFIG_KEY_BINDING_HH
#define CLIENT_CONFIG_KEY_BINDING_HH
#pragma once

#include "core/config/value.hh"

class ConfigKeyBinding final : public ConfigValue {
public:
    explicit ConfigKeyBinding(std::string_view name, SDL_Keycode default_key);
    virtual ~ConfigKeyBinding(void) override = default;

    constexpr operator SDL_Keycode(void) const noexcept;
    constexpr SDL_Keycode keycode(void) const noexcept;
    void set_keycode(SDL_Keycode keycode) noexcept;

    virtual std::string_view value(void) const noexcept override;
    virtual bool set_value(std::string_view value) noexcept override;

private:
    std::string m_string;
    SDL_Keycode m_keycode;
};

constexpr ConfigKeyBinding::operator SDL_Keycode(void) const noexcept
{
    return m_keycode;
}

constexpr SDL_Keycode ConfigKeyBinding::keycode(void) const noexcept
{
    return m_keycode;
}

#endif
