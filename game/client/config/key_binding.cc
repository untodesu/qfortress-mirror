#include "game/client/pch.hh"

#include "game/client/config/key_binding.hh"

ConfigKeyBinding::ConfigKeyBinding(std::string_view name, SDL_Keycode default_key) : ConfigValue(name)
{
    set_keycode(default_key);
}

void ConfigKeyBinding::set_keycode(SDL_Keycode keycode) noexcept
{
    m_keycode = keycode;
    m_string = std::to_string(m_keycode);
}

std::string_view ConfigKeyBinding::value(void) const noexcept
{
    return m_string;
}

bool ConfigKeyBinding::set_value(std::string_view value) noexcept
{
    m_keycode = SDL_GetKeyFromName(value.data());
    m_string = std::to_string(m_keycode);
    return true;
}
