#include "core/pch.hh"

#include "core/paths.hh"

#include "core/cmdline.hh"
#include "core/exceptions.hh"
#include "core/utils/physfs.hh"

static std::filesystem::path s_gamepath;
static std::filesystem::path s_userpath;

const std::filesystem::path& paths::gamepath(void)
{
    return s_gamepath;
}

const std::filesystem::path& paths::userpath(void)
{
    return s_userpath;
}

void paths::init(void)
{
    if(auto value = cmdline::value_or_cstr("gamepath", nullptr)) {
        // If there is a third-party launcher that supports
        // versioning, it may very well just throw in a custom
        // assets directory and the game should consider this
        s_gamepath = std::filesystem::path(value);
    }
    else {
        // Otherwise game assets are always located
        // whereever the game executable is in the assets
        // directory; mind you this follows the git tree structure
        s_gamepath = std::filesystem::current_path() / "assets";
    }

    if(auto value = cmdline::value_or_cstr("userpath", nullptr)) {
        // If there is a third-party launcher that supports
        // multiple profiles, it might throw in a custom user
        // directory with different user-specific configuration files
        s_userpath = std::filesystem::path(value);
    }
    else if(auto windows_appdata = std::getenv("APPDATA")) {
        // We appear to run on a system/environment that is itself
        // a Windows or pretends to be Windows; regardless, persistent
        // application data for the game is stored in %APPDATA% in this case
        s_userpath = std::filesystem::path(windows_appdata) / "QFortress";
    }
    else if(auto xdg_data_home = std::getenv("XDG_DATA_HOME")) {
        // We appear to run on a system/environment that complies
        // or tries to comply with freedesktop spec; by default XDG_DATA_HOME
        // should resolve to ${HOME}/.local/share so it's a good place for userpath
        s_userpath = std::filesystem::path(xdg_data_home) / "qfortress";
    }
    else if(auto unix_home = std::getenv("HOME")) {
        // If anything else fails, as far as I'm concerned, every UNIX system
        // defines an environment variable for user's home directory; we can
        // assume default location from freedesktop spec and put game data there
        s_userpath = std::filesystem::path(unix_home) / ".local/share/qfortress";
    }
    else {
        // Give up and save stuff into cwd
        s_userpath = std::filesystem::current_path();
    }

    LOG_INFO("set gamepath to {}", s_gamepath.string());
    LOG_INFO("set userpath to {}", s_userpath.string());

    std::filesystem::create_directories(s_gamepath);
    std::filesystem::create_directories(s_userpath);

    auto mount_gamepath_ok = PHYSFS_mount(s_gamepath.string().c_str(), nullptr, false);
    qf::throw_if_not_fmt<std::runtime_error>(mount_gamepath_ok, "failed to mount {}: {}", s_gamepath.string(), utils::physfs_error());

    auto mount_userpath_ok = PHYSFS_mount(s_userpath.string().c_str(), nullptr, false);
    qf::throw_if_not_fmt<std::runtime_error>(mount_userpath_ok, "failed to mount {}: {}", s_userpath.string(), utils::physfs_error());

    auto set_write_dir_ok = PHYSFS_setWriteDir(s_userpath.string().c_str());
    qf::throw_if_not_fmt<std::runtime_error>(set_write_dir_ok, "failed to setwritedir {}: {}", s_userpath.string(), utils::physfs_error());
}
