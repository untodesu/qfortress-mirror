#include "client/pch.hh"

#include "client/game.hh"

#include "core/res/resource.hh"

#include "client/res/texture2D.hh"

static res::handle<Texture2D> s_texture;

void client_game::init(void)
{
    // empty
}

void client_game::init_late(void)
{
    s_texture = res::load<Texture2D>("textures/trollface.png", RESFLAG_TEX2D_FLIP);
    assert(s_texture.get());
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
    ImGui::Image(s_texture->imgui, ImVec2(256.0f, 256.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
}
