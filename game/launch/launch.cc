#include "core/pch.hh"

#include "core/cmdline.hh"
#include "core/exceptions.hh"
#include "core/paths.hh"
#include "core/utils/physfs.hh"

#if defined(QF_CLIENT)
#include "game/client/main.hh"
#elif defined(QF_SERVER)
#include "game/server/main.hh"
#else
#error Have your heard of the popular hit game Among Us?
#error Its a really cool game where 1-3 imposters try to kill off the crewmates,
#error while the crew has to finish their tasks or vote off the imposters to win.
#error Its 5 dollars on Steam and consoles but it is free on App Store and Google Play.
#endif

static void wrapped_main(int argc, char** argv)
{
    uulog::add_sink(&uulog::builtin::stderr_ansi);

    cmdline::create(argc, argv);

    auto physfs_init_ok = PHYSFS_init(argv[0]);
    qf::throw_if_not_fmt<std::runtime_error>(physfs_init_ok, "failed to initialize physfs: {}", utils::physfs_error());

    paths::init();

    auto enet_init_fail = static_cast<bool>(enet_initialize());
    qf::throw_if<std::runtime_error>(enet_init_fail, "failed to initialize enet");

#if defined(QF_CLIENT)
    client::main();
#elif defined(QF_SERVER)
    server::main();
#endif

    enet_deinitialize();

    auto physfs_deinit_ok = PHYSFS_deinit();
    qf::throw_if_not_fmt<std::runtime_error>(physfs_deinit_ok, "failed to de-initialize physfs: {}", utils::physfs_error());
}

static void handle_exception(const std::exception* ex)
{
#if defined(QF_CLIENT)
    client::error(ex);
#elif defined(QF_SERVER)
    server::error(ex);
#endif
}

#ifndef NDEBUG
int main(int argc, char** argv)
{
    wrapped_main(argc, argv);
    return EXIT_SUCCESS;
}
#else
int main(int argc, char** argv)
{
    try {
        wrapped_main(argc, argv);
        return EXIT_SUCCESS;
    }
    catch(const std::exception& ex) {
        handle_exception(&ex);
        return EXIT_FAILURE;
    }
    catch(...) {
        handle_exception(nullptr);
        return EXIT_FAILURE;
    }
}
#endif
