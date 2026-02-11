#include "client_modern/pch.hh"

#include "client/render.hh"

#include "core/cmdline.hh"
#include "core/config/boolean.hh"
#include "core/exceptions.hh"

#include "client_modern/globals.hh"
#include "client_modern/utils/shader.hh"

extern const std::uint8_t spirv_triangle_vert[];
extern const std::size_t spirv_triangle_vert_size;

extern const std::uint8_t spirv_triangle_frag[];
extern const std::size_t spirv_triangle_frag_size;

static ConfigBoolean s_vertical_sync("video.vsync", true);

static SDL_GPUCommandBuffer* s_command_buffer;
static SDL_GPUTexture* s_swapchain_texture;
static SDL_GPURenderPass* s_render_pass;
static SDL_GPUGraphicsPipeline* s_pipeline;

void render::init(void)
{
    globals::gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);
    qf::throw_if_not_fmt<std::runtime_error>(globals::gpu_device, "SDL_CreateGPUDevice failed: {}", SDL_GetError());

    LOG_INFO("using SDL_GPU implementation: {}", SDL_GetGPUDeviceDriver(globals::gpu_device));

    auto window_claimed = SDL_ClaimWindowForGPUDevice(globals::gpu_device, globals::window);
    qf::throw_if_not_fmt<std::runtime_error>(window_claimed, "SDL_ClaimWindowForGPUDevice failed: {}", SDL_GetError());
}

void render::init_late(void)
{
    auto vert = utils::create_shader(spirv_triangle_vert, spirv_triangle_vert_size, SDL_GPU_SHADERSTAGE_VERTEX);
    auto frag = utils::create_shader(spirv_triangle_frag, spirv_triangle_frag_size, SDL_GPU_SHADERSTAGE_FRAGMENT);

    SDL_GPUColorTargetDescription colortarget_desc {};
    colortarget_desc.format = SDL_GetGPUSwapchainTextureFormat(globals::gpu_device, globals::window);

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info {};
    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.color_target_descriptions = &colortarget_desc;
    pipeline_info.target_info.has_depth_stencil_target = false;
    pipeline_info.vertex_shader = vert;
    pipeline_info.fragment_shader = frag;
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeline_info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pipeline_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipeline_info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    s_pipeline = SDL_CreateGPUGraphicsPipeline(globals::gpu_device, &pipeline_info);
    qf::throw_if_not_fmt<std::runtime_error>(s_pipeline, "SDL_CreateGPUGraphicsPipeline failed: {}", SDL_GetError());

    SDL_ReleaseGPUShader(globals::gpu_device, frag);
    SDL_ReleaseGPUShader(globals::gpu_device, vert);
}

void render::shutdown(void)
{
    SDL_WaitForGPUIdle(globals::gpu_device);

    SDL_ReleaseGPUGraphicsPipeline(globals::gpu_device, s_pipeline);

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

    if(s_vertical_sync) {
        auto swapchain_acquired = SDL_WaitAndAcquireGPUSwapchainTexture(s_command_buffer, globals::window, &s_swapchain_texture,
            &swapchain_width, &swapchain_height);
        qf::throw_if_not_fmt<std::runtime_error>(swapchain_acquired, "SDL_WaitAndAcquireGPUSwapchainTexture failed: {}", SDL_GetError());
        qf::throw_if_not_fmt<std::runtime_error>(s_swapchain_texture, "swapchain texture is null");
    }
    else {
        auto swapchain_acquired = SDL_AcquireGPUSwapchainTexture(s_command_buffer, globals::window, &s_swapchain_texture, &swapchain_width,
            &swapchain_height);
        qf::throw_if_not_fmt<std::runtime_error>(swapchain_acquired, "SDL_AcquireGPUSwapchainTexture failed: {}", SDL_GetError());
    }

    if(s_swapchain_texture) {
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
}

void render::end_frame(void)
{
    if(s_swapchain_texture) {
        SDL_EndGPURenderPass(s_render_pass);
    }

    SDL_SubmitGPUCommandBuffer(s_command_buffer);
}

void render::render_world(void)
{
    if(s_render_pass) {
        SDL_BindGPUGraphicsPipeline(s_render_pass, s_pipeline);
        SDL_DrawGPUPrimitives(s_render_pass, 3, 1, 0, 0);
    }
}

void render::render_imgui(void)
{
    // empty
}

SDL_WindowFlags render::window_flags(void)
{
    return 0; // SDL_GPU doesn't require any specific flags
}
