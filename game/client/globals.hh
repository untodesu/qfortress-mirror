#ifndef GAME_CLIENT_GLOBALS_HH
#define GAME_CLIENT_GLOBALS_HH
#pragma once

#include "game/shared/globals.hh"

namespace globals
{
extern SDL_Window* window;
} // namespace globals

namespace globals
{
extern std::size_t client_framecount;
extern std::uint64_t client_frametime_us;
extern float client_frametime;
extern float client_frametime_avg;
} // namespace globals

#endif
