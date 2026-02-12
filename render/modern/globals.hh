#ifndef RENDER_MODERN_GLOBALS_HH
#define RENDER_MODERN_GLOBALS_HH
#pragma once

#include "game/client/globals.hh"

namespace globals
{
extern SDL_GPUDevice* gpu_device;
extern SDL_GPUTexture* gpu_swapchain;
extern SDL_GPUCommandBuffer* gpu_commands_main;
} // namespace globals

#endif
