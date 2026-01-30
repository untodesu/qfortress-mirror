#include "client_modern/pch.hh"

#include "client/render.hh"

#include "core/cmdline.hh"
#include "core/config/boolean.hh"
#include "core/exceptions.hh"

#include "client_modern/globals.hh"

static ConfigBoolean s_vertical_sync("video.vsync", true);
static SDL_GPUCommandBuffer* s_command_buffer; // TODO: multithreaded rendering via multiple of these
static SDL_GPUTexture* s_swapchain_texture;
static SDL_GPURenderPass* s_render_pass;

void render::init(void)
{
    globals::gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);
    qf::throw_if_not<std::runtime_error>(globals::gpu_device, "SDL_CreateGPUDevice failed: {}", SDL_GetError());

    LOG_INFO("using SDL_GPU implementation: {}", SDL_GetGPUDeviceDriver(globals::gpu_device));

    auto window_claimed = SDL_ClaimWindowForGPUDevice(globals::gpu_device, globals::window);
    qf::throw_if_not<std::runtime_error>(window_claimed, "SDL_ClaimWindowForGPUDevice failed: {}", SDL_GetError());
}

void render::init_late(void)
{
    // empty
}

void render::shutdown(void)
{
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
    qf::throw_if_not<std::runtime_error>(s_command_buffer, "SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());

    Uint32 swapchain_width;
    Uint32 swapchain_height;
    auto swapchain_acquired = SDL_WaitAndAcquireGPUSwapchainTexture(s_command_buffer, globals::window, &s_swapchain_texture,
        &swapchain_width, &swapchain_height);
    qf::throw_if_not<std::runtime_error>(swapchain_acquired, "SDL_AcquireGPUSwapchainTexture failed: {}", SDL_GetError());
    qf::throw_if_not<std::runtime_error>(s_swapchain_texture, "swapchain texture is null");

    SDL_GPUColorTargetInfo targetinfo {};
    targetinfo.texture = s_swapchain_texture;
    targetinfo.cycle = true;
    targetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
    targetinfo.store_op = SDL_GPU_STOREOP_STORE;
    targetinfo.clear_color.r = 0.0f;
    targetinfo.clear_color.g = 0.0f;
    targetinfo.clear_color.b = 0.1f;
    targetinfo.clear_color.a = 0.0f;

    s_render_pass = SDL_BeginGPURenderPass(s_command_buffer, &targetinfo, 1, nullptr);
}

void render::end_frame(void)
{
    SDL_EndGPURenderPass(s_render_pass);
    SDL_SubmitGPUCommandBuffer(s_command_buffer);
}

void render::render_world(void)
{
    // empty
}

void render::render_imgui(void)
{
    // empty
}

SDL_WindowFlags render::window_flags(void)
{
    return 0; // SDL_GPU doesn't require any specific flags
}
