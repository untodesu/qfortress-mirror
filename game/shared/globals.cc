#include "game/shared/pch.hh"

#include "game/shared/globals.hh"

entt::dispatcher globals::dispatcher;
entt::registry globals::registry;

std::uint64_t globals::curtime_us;

std::size_t globals::fixed_framecount;
std::uint64_t globals::fixed_frametime_us;
float globals::fixed_frametime;
float globals::fixed_frametime_avg;
