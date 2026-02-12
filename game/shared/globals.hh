#ifndef GAME_SHARED_GLOBALS_HH
#define GAME_SHARED_GLOBALS_HH
#pragma once

namespace globals
{
extern entt::dispatcher dispatcher;
extern entt::registry registry;
} // namespace globals

namespace globals
{
extern std::uint64_t curtime_us;
} // namespace globals

namespace globals
{
extern std::size_t fixed_framecount;
extern std::uint64_t fixed_frametime_us;
extern float fixed_frametime;
extern float fixed_frametime_avg;
} // namespace globals

#endif
