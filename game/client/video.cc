#include "game/client/pch.hh"

#include "game/client/video.hh"

#include "core/exceptions.hh"

#include "game/client/globals.hh"

#include "render/backend.hh"

void video::init(void)
{
    qf::throw_if_not_fmt<std::runtime_error>(SDL_Init(SDL_INIT_VIDEO), "SDL_Init for video subsystem failed: {}", SDL_GetError());

    globals::window = SDL_CreateWindow("client", 640, 480, SDL_WINDOW_RESIZABLE | render_backend::window_flags());
    qf::throw_if_not_fmt<std::runtime_error>(globals::window, "SDL_CreateWindow failed: {}", SDL_GetError());
}

void video::init_late(void)
{
}

void video::shutdown(void)
{
    SDL_DestroyWindow(globals::window);
}
