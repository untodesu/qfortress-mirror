#include "render/modern/pch.hh"

#include "render/modern/globals.hh"

SDL_GPUDevice* globals::gpu_device = nullptr;
SDL_GPUTexture* globals::gpu_swapchain = nullptr;
SDL_GPUCommandBuffer* globals::gpu_commands_main = nullptr;
