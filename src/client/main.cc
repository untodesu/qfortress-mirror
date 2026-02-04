#include "client/pch.hh"

#include "client/main.hh"

#include "core/exceptions.hh"
#include "core/utils/epoch.hh"
#include "core/version.hh"

#include "client/globals.hh"
#include "client/render.hh"
#include "client/video.hh"

static std::atomic_bool s_is_running;

static void signal_handler(int)
{
    LOG_INFO("received termination signal");

    s_is_running.store(false);
}

static void handle_events(void)
{
    thread_local SDL_Event event;

    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_EVENT_QUIT) {
            s_is_running.store(false);
            return;
        }

        globals::dispatcher.trigger(static_cast<const SDL_Event&>(event));
    }
}

void client::main(void)
{
    std::signal(SIGINT, &signal_handler);
    std::signal(SIGTERM, &signal_handler);

    qf::throw_if_not_fmt<std::runtime_error>(SDL_Init(SDL_INIT_EVENTS), "SDL_Init for events subsystem failed: {}", SDL_GetError());

    video::init();
    render::init();

    video::init_late();
    render::init_late();

    s_is_running.store(true);

    globals::curtime_us = utils::epoch_microseconds();

    globals::client_framecount = 0;
    globals::client_frametime_us = 0;
    globals::client_frametime = 0.0f;
    globals::client_frametime_avg = 0.0f;

    auto last_curtime_us = globals::curtime_us;

    while(s_is_running.load()) {
        globals::curtime_us = utils::epoch_microseconds();

        globals::client_frametime_us = globals::curtime_us - last_curtime_us;
        globals::client_frametime = 1.0e-6f * static_cast<float>(globals::client_frametime_us);
        globals::client_frametime_avg += globals::client_frametime;
        globals::client_frametime_avg *= 0.5f;

        last_curtime_us = globals::curtime_us;

        handle_events();

        render::begin_frame();

        render::render_world();

        render::render_imgui();

        render::end_frame();

        globals::client_framecount += 1;
    }

    LOG_INFO("client shutdown after {} frames", globals::client_framecount);
    LOG_INFO("average framerate: {:.03f} FPS ({:.03f} ms)", 1.0f / globals::client_frametime_avg, 1000.0f * globals::client_frametime_avg);

    render::shutdown();
    video::shutdown();
}

void client::error(const std::exception* ex)
{
    if(ex == nullptr) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Engine Error: non-std::exception throw");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Error", "Non-std::exception throw", globals::window);
    }
    else {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Engine Error: %s", ex->what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Error", ex->what(), globals::window);
    }
}
