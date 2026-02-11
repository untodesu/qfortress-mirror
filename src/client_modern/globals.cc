#include "client_modern/pch.hh"

#include "client_modern/globals.hh"

SDL_GPUDevice* globals::gpu_device = nullptr;
SDL_GPUTexture* globals::gpu_swapchain = nullptr;
SDL_GPURenderPass* globals::gpu_render_pass = nullptr;
