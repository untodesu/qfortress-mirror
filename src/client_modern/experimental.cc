#include "client_modern/pch.hh"

#include "client_modern/experimental.hh"

#include "core/exceptions.hh"

#include "client_modern/globals.hh"
#include "client_modern/utils/shader.hh"

extern const std::uint8_t spirv_triangle_vert[];
extern const std::size_t spirv_triangle_vert_size;

extern const std::uint8_t spirv_triangle_frag[];
extern const std::size_t spirv_triangle_frag_size;

static SDL_GPUGraphicsPipeline* s_pipeline;

void experimental::init(void)
{
    // empty
}

void experimental::init_late(void)
{
    auto vert = utils::create_shader(spirv_triangle_vert, spirv_triangle_vert_size, SDL_GPU_SHADERSTAGE_VERTEX);
    auto frag = utils::create_shader(spirv_triangle_frag, spirv_triangle_frag_size, SDL_GPU_SHADERSTAGE_FRAGMENT);

    SDL_GPUColorTargetDescription color_target_desc {};
    color_target_desc.format = SDL_GetGPUSwapchainTextureFormat(globals::gpu_device, globals::window);

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info {};

    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.color_target_descriptions = &color_target_desc;

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

void experimental::shutdown(void)
{
    SDL_ReleaseGPUGraphicsPipeline(globals::gpu_device, s_pipeline);
}

void experimental::render_world(void)
{
    SDL_BindGPUGraphicsPipeline(globals::gpu_render_pass, s_pipeline);
    SDL_DrawGPUPrimitives(globals::gpu_render_pass, 3, 1, 0, 0);
}

void experimental::render_imgui(void)
{
    // empty
}
