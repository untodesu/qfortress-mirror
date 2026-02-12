#include "game/client/pch.hh"

#include "game/client/game.hh"

#include "core/resource.hh"

#include "game/client/globals.hh"

#include "render/texture2d.hh"

static res::handle<Texture2D> s_texture;

static void on_sdl_key(const SDL_KeyboardEvent& event)
{
    if(event.type == SDL_EVENT_KEY_DOWN && event.key == SDLK_ESCAPE) {
        std::raise(SIGINT);
    }
}

void client_game::init(void)
{
    globals::dispatcher.sink<SDL_KeyboardEvent>().connect<&on_sdl_key>();
}

void client_game::init_late(void)
{
    s_texture = res::load<Texture2D>("textures/trollface.png");
}

void client_game::shutdown(void)
{
    s_texture.reset();
}

void client_game::update(void)
{
    // empty
}

void client_game::update_late(void)
{
    // empty
}

void client_game::fixed_update(void)
{
    // empty
}

void client_game::fixed_update_late(void)
{
    // empty
}

void client_game::layout(void)
{
    ImGui::Image(s_texture->imgui, ImVec2(256.0f, 196.0f));
}
