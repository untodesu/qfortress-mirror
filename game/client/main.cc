#include "game/client/pch.hh"

#include "game/client/main.hh"

#include "core/config/map.hh"
#include "core/entity/current_leaf.hh"
#include "core/entity/transform.hh"
#include "core/exceptions.hh"
#include "core/image.hh"
#include "core/level/level.hh"
#include "core/resource.hh"
#include "core/utils/epoch.hh"
#include "core/version.hh"

#include "game/client/game.hh"
#include "game/client/globals.hh"
#include "game/client/video.hh"

#include "render/backend.hh"
#include "render/frontend.hh"
#include "render/texture2d.hh"

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

        switch(event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                globals::dispatcher.trigger(static_cast<const SDL_KeyboardEvent&>(event.key));
                break;

            case SDL_EVENT_MOUSE_MOTION:
                globals::dispatcher.trigger(static_cast<const SDL_MouseMotionEvent&>(event.motion));
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                globals::dispatcher.trigger(static_cast<const SDL_MouseButtonEvent&>(event.button));
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                globals::dispatcher.trigger(static_cast<const SDL_MouseWheelEvent&>(event.wheel));
                break;

            case SDL_EVENT_WINDOW_MOVED:
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
            case SDL_EVENT_WINDOW_MINIMIZED:
            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            case SDL_EVENT_WINDOW_FOCUS_LOST:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            case SDL_EVENT_WINDOW_HIT_TEST:
            case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
            case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
            case SDL_EVENT_WINDOW_OCCLUDED:
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            case SDL_EVENT_WINDOW_DESTROYED:
            case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
                globals::dispatcher.trigger(static_cast<const SDL_WindowEvent&>(event.window));
                break;
        }

        globals::dispatcher.trigger(static_cast<const SDL_Event&>(event));
    }
}

void client::main(void)
{
    std::signal(SIGINT, &signal_handler);
    std::signal(SIGTERM, &signal_handler);

    qf::throw_if_not_fmt<std::runtime_error>(SDL_Init(SDL_INIT_EVENTS), "SDL_Init for events subsystem failed: {}", SDL_GetError());

    Image::register_resource();
    Texture2D::register_resource();

    video::init();
    render_backend::init();
    render_frontend::init();
    client_game::init();

    globals::client_config.load("client.conf");
    globals::client_config.load("client.user.conf");

    video::init_late();
    render_backend::init_late();
    render_frontend::init_late();
    client_game::init_late();

    Transform::register_component();
    CurrentLeaf::register_component();

    Level test_write;
    auto& test_write_r = test_write.registry();
    auto test_write_ent = test_write_r.create();
    test_write_r.emplace_or_replace<Transform>(test_write_ent, Transform({ 123.0f, 456.0f, 789.0f }, Eigen::Quaternionf::Identity()));

    test_write.save("testlevel.bsp");
    test_write.purge();

    Level test_read;
    test_read.load("testlevel.bsp");

    auto& test_read_r = test_read.registry();
    auto test_read_ent = test_read_r.view<entt::entity>().front();
    auto& test_read_transform = test_read_r.get<Transform>(test_read_ent);
    Eigen::Vector3f test_read_pos(test_read_transform.position());
    Eigen::Vector3f test_read_ijk(test_read_transform.forward_vector());
    LOG_INFO("pos {} {} {}", test_read_pos.x(), test_read_pos.y(), test_read_pos.z());
    LOG_INFO("fwd {} {} {}", test_read_ijk.x(), test_read_ijk.y(), test_read_ijk.z());

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

        render_frontend::update();
        client_game::update();

        render_backend::prepare();

        render_frontend::render();

        client_game::layout();
        render_frontend::layout();

        render_backend::present();

        render_frontend::update_late();
        client_game::update_late();

        globals::client_framecount += 1;

        res::soft_purge();
    }

    LOG_INFO("client shutdown after {} frames", globals::client_framecount);
    LOG_INFO("average framerate: {:.03f} FPS ({:.03f} ms)", 1.0f / globals::client_frametime_avg, 1000.0f * globals::client_frametime_avg);

    client_game::shutdown();
    render_frontend::shutdown();

    res::hard_purge();

    render_backend::shutdown();
    video::shutdown();

    globals::client_config.save("client.conf");
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
