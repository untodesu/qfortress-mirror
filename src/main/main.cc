#include "core/pch.hh"

#include "core/cmdline.hh"
#include "core/exceptions.hh"
#include "core/utils/physfs.hh"

#if defined(QF_CLIENT)
#include "client/main.hh"
#elif defined(QF_SERVER)
#include "server/main.hh"
#else
#error Have your heard of the popular hit game Among Us?
#error Its a really cool game where 1-3 imposters try to kill off the crewmates,
#error while the crew has to finish their tasks or vote off the imposters to win.
#error Its 5 dollars on Steam and consoles but it is free on App Store and Google Play.
#endif

static std::filesystem::path get_gamepath(void)
{
    if(auto value = cmdline::value_or_cstr("gamepath", nullptr)) {
        // If there is a third-party launcher that supports
        // versioning, it may very well just throw in a custom
        // assets directory and the game should consider this
        return std::filesystem::path(value);
    }

    // Otherwise game assets are always located
    // whereever the game executable is in the assets
    // directory; mind you this follows the git tree structure
    return std::filesystem::current_path() / "assets";
}

static std::filesystem::path get_userpath(void)
{
    if(auto value = cmdline::value_or_cstr("userpath", nullptr)) {
        // If there is a third-party launcher that supports
        // multiple profiles, it might throw in a custom user
        // directory with different user-specific configuration files
        return std::filesystem::path(value);
    }

    if(auto windows_appdata = std::getenv("APPDATA")) {
        // We appear to run on a system/environment that is itself
        // a Windows or pretends to be Windows; regardless, persistent
        // application data for the game is stored in %APPDATA% in this case
        return std::filesystem::path(windows_appdata) / "QFortress";
    }

    if(auto xdg_data_home = std::getenv("XDG_DATA_HOME")) {
        // We appear to run on a system/environment that complies
        // or tries to comply with freedesktop spec; by default XDG_DATA_HOME
        // should resolve to ${HOME}/.local/share so it's a good place for userpath
        return std::filesystem::path(xdg_data_home) / "qfortress";
    }

    if(auto unix_home = std::getenv("HOME")) {
        // If anything else fails, as far as I'm concerned, every UNIX system
        // defines an environment variable for user's home directory; we can
        // assume default location from freedesktop spec and put game data there
        return std::filesystem::path(unix_home) / ".local/share/qfortress";
    }

    return std::filesystem::current_path(); // Give up and save stuff into cwd
}

static void wrapped_main(int argc, char** argv)
{
    uulog::add_sink(&uulog::builtin::stderr_ansi);

    cmdline::create(argc, argv);

    if(!PHYSFS_init(argv[0])) {
        throw qf::runtime_error("physfs init failed: {}", utils::physfs_error());
    }

    auto gamepath = get_gamepath();
    auto userpath = get_userpath();

    LOG_INFO("set gamepath to {}", gamepath.string());
    LOG_INFO("set userpath to {}", userpath.string());

    std::filesystem::create_directories(gamepath);
    std::filesystem::create_directories(userpath);

    if(!PHYSFS_mount(gamepath.string().c_str(), nullptr, false)) {
        throw qf::runtime_error("failed to mount {}: {}", gamepath.string(), utils::physfs_error());
    }

    if(!PHYSFS_mount(userpath.string().c_str(), nullptr, false)) {
        throw qf::runtime_error("failed to mount {}: {}", userpath.string(), utils::physfs_error());
    }

    if(!PHYSFS_setWriteDir(userpath.string().c_str())) {
        throw qf::runtime_error("failed to setwritedir {}: {}", userpath.string(), utils::physfs_error());
    }

    if(enet_initialize()) {
        throw qf::runtime_error("enet init failed");
    }

#if defined(QF_CLIENT)
    client::main();
#elif defined(QF_SERVER)
    server::main();
#endif

    enet_deinitialize();

    if(!PHYSFS_deinit()) {
        throw qf::runtime_error("physfs deinit failed: {}", utils::physfs_error());
    }
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
