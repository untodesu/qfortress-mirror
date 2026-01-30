#include "client/pch.hh"

#include "client/main.hh"

#include "core/exceptions.hh"

void client::main(void)
{
    if(!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        throw qf::runtime_error("SDL_Init failed: {}", SDL_GetError());
    }

    auto window = SDL_CreateWindow("client", 640, 480, SDL_WINDOW_RESIZABLE);

    if(window == nullptr) {
        throw qf::runtime_error("SDL_CreateWindow failed: {}", SDL_GetError());
    }

    bool is_running = true;

    while(is_running) {
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT:
                    is_running = false;
                    break;
            }
        }
    }

    SDL_DestroyWindow(window);
}
