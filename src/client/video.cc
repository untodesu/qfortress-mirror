#include "client/pch.hh"

#include "client/video.hh"

#include "core/exceptions.hh"

#include "client/globals.hh"
#include "client/render.hh"

static void on_sdl_event(const SDL_Event& event)
{
    if(event.type == SDL_EVENT_WINDOW_RESIZED) {
        globals::dispatcher.trigger(WindowResizeEvent(event.window.data1, event.window.data2));
    }
}

void video::init(void)
{
    qf::throw_if_not_fmt<std::runtime_error>(SDL_Init(SDL_INIT_VIDEO), "SDL_Init for video subsystem failed: {}", SDL_GetError());

    globals::window = SDL_CreateWindow("client", 640, 480, SDL_WINDOW_RESIZABLE | render::window_flags());
    qf::throw_if_not_fmt<std::runtime_error>(globals::window, "SDL_CreateWindow failed: {}", SDL_GetError());

    globals::dispatcher.sink<SDL_Event>().connect<&on_sdl_event>();
}

void video::init_late(void)
{
}

void video::shutdown(void)
{
    SDL_DestroyWindow(globals::window);
}
