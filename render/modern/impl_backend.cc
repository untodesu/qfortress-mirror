#include "render/modern/pch.hh"

#include "render/backend.hh"

#include "core/cmdline.hh"
#include "core/exceptions.hh"

#include "render/modern/globals.hh"

static void on_sdl_event(const SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
}

static void do_imgui_render_pass(void)
{
    ImGui::Render();

    auto draw_data = ImGui::GetDrawData();

    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, globals::gpu_commands_main);

    SDL_GPUColorTargetInfo color_target_info = {};
    color_target_info.texture = globals::gpu_swapchain;
    color_target_info.load_op = SDL_GPU_LOADOP_LOAD; // draw over the rest
    color_target_info.store_op = SDL_GPU_STOREOP_STORE;

    auto render_pass = SDL_BeginGPURenderPass(globals::gpu_commands_main, &color_target_info, 1, nullptr);
    qf::throw_if_not<std::runtime_error>(render_pass, "SDL_BeginGPURenderPass returned nullptr");

    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, globals::gpu_commands_main, render_pass);

    SDL_EndGPURenderPass(render_pass);
}

void render_backend::init(void)
{
    globals::gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);
    qf::throw_if_not_fmt<std::runtime_error>(globals::gpu_device, "failed to create a GPU device: {}", SDL_GetError());

    LOG_INFO("using SDL_GPU implementation: {}", SDL_GetGPUDeviceDriver(globals::gpu_device));

    auto window_claimed = SDL_ClaimWindowForGPUDevice(globals::gpu_device, globals::window);
    qf::throw_if_not_fmt<std::runtime_error>(window_claimed, "failed to claim an SDL window for GPU operations: {}", SDL_GetError());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto imgui_winit_ok = ImGui_ImplSDL3_InitForSDLGPU(globals::window);
    qf::throw_if_not<std::runtime_error>(imgui_winit_ok, "failed to initialize ImGui for SDL3 backend");

    ImGui_ImplSDLGPU3_InitInfo imgui_ginit_info {};
    imgui_ginit_info.Device = globals::gpu_device;
    imgui_ginit_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(globals::gpu_device, globals::window);

    auto imgui_ginit_ok = ImGui_ImplSDLGPU3_Init(&imgui_ginit_info);
    qf::throw_if_not<std::runtime_error>(imgui_winit_ok, "failed to initialize ImGui for SDL3_GPU backend");

    globals::dispatcher.sink<SDL_Event>().connect<&on_sdl_event>();
}

void render_backend::init_late(void)
{
    // empty
}

void render_backend::shutdown(void)
{
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();

    SDL_ReleaseWindowFromGPUDevice(globals::gpu_device, globals::window);
    SDL_DestroyGPUDevice(globals::gpu_device);
}

void render_backend::prepare(void)
{
    globals::gpu_commands_main = SDL_AcquireGPUCommandBuffer(globals::gpu_device);
    qf::throw_if_not_fmt<std::runtime_error>(globals::gpu_commands_main, "failed to acquire a GPU command buffer: {}", SDL_GetError());

    Uint32 swapchain_width;
    Uint32 swapchain_height;

    auto swapchain_acquired = SDL_WaitAndAcquireGPUSwapchainTexture(globals::gpu_commands_main, globals::window, &globals::gpu_swapchain,
        &swapchain_width, &swapchain_height);
    qf::throw_if_not_fmt<std::runtime_error>(swapchain_acquired, "failed to acquire a GPU swapchain texture: {}", SDL_GetError());
    qf::throw_if_not_fmt<std::runtime_error>(globals::gpu_swapchain, "SDL_WaitAndAcquireGPUSwapchainTexture returned nullptr");

    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void render_backend::present(void)
{
    do_imgui_render_pass();

    SDL_SubmitGPUCommandBuffer(globals::gpu_commands_main);
}

SDL_WindowFlags render_backend::window_flags(void)
{
    return 0; // SDL_GPU doesn't require any specific flags
}
