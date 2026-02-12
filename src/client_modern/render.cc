#include "client_modern/pch.hh"

#include "client/render.hh"

#include "core/cmdline.hh"
#include "core/exceptions.hh"

#include "client_modern/experimental.hh"
#include "client_modern/globals.hh"

static SDL_GPUCommandBuffer* s_command_buffer;

void render::init(void)
{
    globals::gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);
    qf::throw_if_not_fmt<std::runtime_error>(globals::gpu_device, "failed to create a GPU device: {}", SDL_GetError());

    LOG_INFO("using SDL_GPU implementation: {}", SDL_GetGPUDeviceDriver(globals::gpu_device));

    auto window_claimed = SDL_ClaimWindowForGPUDevice(globals::gpu_device, globals::window);
    qf::throw_if_not_fmt<std::runtime_error>(window_claimed, "failed to claim an SDL window for GPU operations: {}", SDL_GetError());

    experimental::init();
}

void render::init_late(void)
{
    experimental::init_late();
}

void render::shutdown(void)
{
    SDL_WaitForGPUIdle(globals::gpu_device);

    experimental::shutdown();

    SDL_ReleaseWindowFromGPUDevice(globals::gpu_device, globals::window);
    SDL_DestroyGPUDevice(globals::gpu_device);
}

void render::update(void)
{
    experimental::update();
}

void render::update_late(void)
{
    experimental::update_late();
}

void render::begin_frame(void)
{
    s_command_buffer = SDL_AcquireGPUCommandBuffer(globals::gpu_device);
    qf::throw_if_not_fmt<std::runtime_error>(s_command_buffer, "failed to acquire a GPU command buffer: {}", SDL_GetError());

    Uint32 swapchain_width;
    Uint32 swapchain_height;

    auto swapchain_acquired = SDL_WaitAndAcquireGPUSwapchainTexture(s_command_buffer, globals::window, &globals::gpu_swapchain,
        &swapchain_width, &swapchain_height);
    qf::throw_if_not_fmt<std::runtime_error>(swapchain_acquired, "failed to acquire a GPU swapchain texture: {}", SDL_GetError());
    qf::throw_if_not_fmt<std::runtime_error>(globals::gpu_swapchain, "SDL_WaitAndAcquireGPUSwapchainTexture returned nullptr");
}

void render::end_frame(void)
{
    SDL_SubmitGPUCommandBuffer(s_command_buffer);
}

void render::render(void)
{
    experimental::render(s_command_buffer);
}

void render::layout(void)
{
    // empty
}

SDL_WindowFlags render::window_flags(void)
{
    return 0; // SDL_GPU doesn't require any specific flags
}
