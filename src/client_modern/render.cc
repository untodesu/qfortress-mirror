#include "client_modern/pch.hh"

#include "client/render.hh"

#include "core/cmdline.hh"
#include "core/exceptions.hh"

#include "client_modern/experimental.hh"
#include "client_modern/globals.hh"

static SDL_GPUPresentMode s_present_mode_vsync;
static SDL_GPUCommandBuffer* s_command_buffer;

void render::init(void)
{
    globals::gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);
    qf::throw_if_not_fmt<std::runtime_error>(globals::gpu_device, "SDL_CreateGPUDevice failed: {}", SDL_GetError());

    LOG_INFO("using SDL_GPU implementation: {}", SDL_GetGPUDeviceDriver(globals::gpu_device));

    auto window_claimed = SDL_ClaimWindowForGPUDevice(globals::gpu_device, globals::window);
    qf::throw_if_not_fmt<std::runtime_error>(window_claimed, "SDL_ClaimWindowForGPUDevice failed: {}", SDL_GetError());

    if(SDL_WindowSupportsGPUPresentMode(globals::gpu_device, globals::window, SDL_GPU_PRESENTMODE_MAILBOX)) {
        s_present_mode_vsync = SDL_GPU_PRESENTMODE_MAILBOX;
    }
    else {
        s_present_mode_vsync = SDL_GPU_PRESENTMODE_VSYNC;
    }

    render::request_vsync(false);

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
    // empty
}

void render::update_late(void)
{
    // empty
}

void render::begin_frame(void)
{
    s_command_buffer = SDL_AcquireGPUCommandBuffer(globals::gpu_device);
    qf::throw_if_not_fmt<std::runtime_error>(s_command_buffer, "SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());

    Uint32 swapchain_width;
    Uint32 swapchain_height;

    auto swapchain_acquired = SDL_WaitAndAcquireGPUSwapchainTexture(s_command_buffer, globals::window, &globals::gpu_swapchain,
        &swapchain_width, &swapchain_height);
    qf::throw_if_not_fmt<std::runtime_error>(swapchain_acquired, "SDL_WaitAndAcquireGPUSwapchainTexture failed: {}", SDL_GetError());
    qf::throw_if_not_fmt<std::runtime_error>(globals::gpu_swapchain, "swapchain texture is null");

    SDL_GPUColorTargetInfo target_info {};
    target_info.texture = globals::gpu_swapchain;
    target_info.cycle = true;
    target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    target_info.store_op = SDL_GPU_STOREOP_STORE;
    target_info.clear_color.r = 0.0f;
    target_info.clear_color.g = 0.0f;
    target_info.clear_color.b = 0.1f;
    target_info.clear_color.a = 0.0f;

    globals::gpu_render_pass = SDL_BeginGPURenderPass(s_command_buffer, &target_info, 1, nullptr);
}

void render::end_frame(void)
{
    SDL_EndGPURenderPass(globals::gpu_render_pass);
    SDL_SubmitGPUCommandBuffer(s_command_buffer);
}

void render::render_world(void)
{
    experimental::render_world();
}

void render::render_imgui(void)
{
    experimental::render_imgui();
}

void render::request_vsync(bool vsync)
{
    auto mode = vsync ? s_present_mode_vsync : SDL_GPU_PRESENTMODE_IMMEDIATE;
    auto ok = SDL_SetGPUSwapchainParameters(globals::gpu_device, globals::window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, mode);
    qf::throw_if_not_fmt<std::runtime_error>(ok, "SDL_SetGPUSwapchainParameters failed: {}", SDL_GetError());
}

SDL_WindowFlags render::window_flags(void)
{
    return 0; // SDL_GPU doesn't require any specific flags
}
