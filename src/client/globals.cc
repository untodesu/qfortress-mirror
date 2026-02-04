#include "client/pch.hh"

#include "client/globals.hh"

SDL_Window* globals::window = nullptr;

std::size_t globals::client_framecount;
std::uint64_t globals::client_frametime_us;
float globals::client_frametime;
float globals::client_frametime_avg;
