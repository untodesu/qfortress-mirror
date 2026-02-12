#include "client_modern/pch.hh"

#include "client_modern/experimental.hh"

#include "core/exceptions.hh"
#include "core/math/camera.hh"
#include "core/res/resource.hh"

#include "client/res/texture2D.hh"
#include "client/video.hh"

#include "client_modern/globals.hh"
#include "client_modern/utils/static_buffer.hh"

extern const std::uint8_t spirv_experimental_vert[];
extern const std::size_t spirv_experimental_vert_size;

extern const std::uint8_t spirv_experimental_frag[];
extern const std::size_t spirv_experimental_frag_size;

struct Vertex final {
    Eigen::Vector3f position;
    Eigen::Vector2f texcoord;
};

struct Uniforms final {
    alignas(16) Eigen::Matrix4f mvp;
};

static SDL_GPUGraphicsPipeline* s_pipeline;
static std::unique_ptr<utils::StaticBuffer> s_vbo;
static std::unique_ptr<utils::StaticBuffer> s_ibo;

static res::handle<Texture2D> s_texture;
static SDL_GPUSampler* s_sampler;

static math::Camera s_camera;
static float s_phase;

void experimental::init(void)
{
    s_phase = 0.0f;
}

void experimental::init_late(void)
{
    SDL_GPUShaderCreateInfo vert_info {};
    vert_info.code_size = spirv_experimental_vert_size;
    vert_info.code = reinterpret_cast<const Uint8*>(spirv_experimental_vert);
    vert_info.entrypoint = "main";
    vert_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    vert_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    vert_info.num_uniform_buffers = 1;

    SDL_GPUShaderCreateInfo frag_info {};
    frag_info.code_size = spirv_experimental_frag_size;
    frag_info.code = reinterpret_cast<const Uint8*>(spirv_experimental_frag);
    frag_info.entrypoint = "main";
    frag_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    frag_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    frag_info.num_samplers = 1;

    auto vert = SDL_CreateGPUShader(globals::gpu_device, &vert_info);
    qf::throw_if_not_fmt<std::runtime_error>(vert, "failed to create a vertex shader: {}", SDL_GetError());

    auto frag = SDL_CreateGPUShader(globals::gpu_device, &frag_info);
    qf::throw_if_not_fmt<std::runtime_error>(frag, "failed to create a fragment shader: {}", SDL_GetError());

    std::vector<Vertex> vertices;
    vertices.push_back({ Eigen::Vector3f(-0.5f, -0.5f, 0.0f), Eigen::Vector2f(0.0f, 0.0f) });
    vertices.push_back({ Eigen::Vector3f(-0.5f, +0.5f, 0.0f), Eigen::Vector2f(0.0f, 1.0f) });
    vertices.push_back({ Eigen::Vector3f(+0.5f, +0.5f, 0.0f), Eigen::Vector2f(1.0f, 1.0f) });
    vertices.push_back({ Eigen::Vector3f(+0.5f, -0.5f, 0.0f), Eigen::Vector2f(1.0f, 0.0f) });

    std::vector<Uint32> indices;
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);

    s_vbo = std::make_unique<utils::StaticBuffer>(sizeof(Vertex) * vertices.size(), SDL_GPU_BUFFERUSAGE_VERTEX);
    s_vbo->upload<Vertex>(vertices);

    s_ibo = std::make_unique<utils::StaticBuffer>(sizeof(Uint32) * indices.size(), SDL_GPU_BUFFERUSAGE_INDEX);
    s_ibo->upload<Uint32>(indices);

    SDL_GPUColorTargetDescription color_target_desc {};
    color_target_desc.format = SDL_GetGPUSwapchainTextureFormat(globals::gpu_device, globals::window);

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info {};
    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.color_target_descriptions = &color_target_desc;

    SDL_GPUVertexBufferDescription vertex_buffer_desc {};
    vertex_buffer_desc.slot = 0;
    vertex_buffer_desc.pitch = static_cast<Uint32>(sizeof(Vertex));
    vertex_buffer_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertex_buffer_desc.instance_step_rate = 0;

    SDL_GPUVertexAttribute attr_position {};
    attr_position.location = 0;
    attr_position.buffer_slot = 0;
    attr_position.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attr_position.offset = offsetof(Vertex, position);

    SDL_GPUVertexAttribute attr_texcoord {};
    attr_texcoord.location = 1;
    attr_texcoord.buffer_slot = 0;
    attr_texcoord.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attr_texcoord.offset = offsetof(Vertex, texcoord);

    std::vector<SDL_GPUVertexAttribute> vertex_attributes;
    vertex_attributes.emplace_back(std::move(attr_position));
    vertex_attributes.emplace_back(std::move(attr_texcoord));

    pipeline_info.vertex_input_state.num_vertex_buffers = 1;
    pipeline_info.vertex_input_state.vertex_buffer_descriptions = &vertex_buffer_desc;
    pipeline_info.vertex_input_state.num_vertex_attributes = static_cast<Uint32>(vertex_attributes.size());
    pipeline_info.vertex_input_state.vertex_attributes = vertex_attributes.data();

    pipeline_info.vertex_shader = vert;
    pipeline_info.fragment_shader = frag;

    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeline_info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pipeline_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipeline_info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    s_pipeline = SDL_CreateGPUGraphicsPipeline(globals::gpu_device, &pipeline_info);
    qf::throw_if_not_fmt<std::runtime_error>(s_pipeline, "failed to create a GPU graphics pipeline: {}", SDL_GetError());

    SDL_ReleaseGPUShader(globals::gpu_device, frag);
    SDL_ReleaseGPUShader(globals::gpu_device, vert);

    s_texture = res::load<Texture2D>("textures/trollface.png", RESFLAG_TEX2D_FLIP);
    qf::throw_if_not<std::runtime_error>(s_texture.get(), "failed to load a texture");

    SDL_GPUSamplerCreateInfo sampler_info {};
    sampler_info.min_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.mag_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;

    s_sampler = SDL_CreateGPUSampler(globals::gpu_device, &sampler_info);
    qf::throw_if_not_fmt<std::runtime_error>(s_sampler, "failed to create a GPU sampler: {}", SDL_GetError());
}

void experimental::shutdown_early(void)
{
    SDL_ReleaseGPUSampler(globals::gpu_device, s_sampler);

    s_texture.reset();

    s_ibo.reset();
    s_vbo.reset();

    SDL_ReleaseGPUGraphicsPipeline(globals::gpu_device, s_pipeline);
}

void experimental::update(void)
{
    s_phase += globals::client_frametime;

    auto freq = s_phase * 2.0f * float(M_PI);
    auto sval = std::sinf(freq);
    auto cval = std::cosf(freq);

    s_camera.set_projection_perspective(float(M_PI_2), video::aspect, 0.01f, 200.0f);
    s_camera.set_look(Eigen::Vector3f(sval, sval, cval), Eigen::Vector3f::Zero());
    s_camera.update();
}

void experimental::update_late(void)
{
    // empty
}

void experimental::render(SDL_GPUCommandBuffer* command_buffer)
{
    assert(command_buffer);

    // Copy pass
    // empty

    // Render pass

    SDL_GPUColorTargetInfo target_info {};
    target_info.texture = globals::gpu_swapchain;
    target_info.cycle = true;
    target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    target_info.store_op = SDL_GPU_STOREOP_STORE;
    target_info.clear_color.r = 0.0f;
    target_info.clear_color.g = 0.0f;
    target_info.clear_color.b = 0.1f;
    target_info.clear_color.a = 0.0f;

    auto render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);
    qf::throw_if_not<std::runtime_error>(render_pass, "SDL_BeginGPURenderPass returned nullptr");

    SDL_BindGPUGraphicsPipeline(render_pass, s_pipeline);

    SDL_GPUTextureSamplerBinding texture_binding {};
    texture_binding.texture = reinterpret_cast<SDL_GPUTexture*>(s_texture->gpu_handle);
    texture_binding.sampler = s_sampler;

    SDL_GPUBufferBinding vbo_binding {};
    vbo_binding.buffer = s_vbo->handle();
    vbo_binding.offset = 0;

    SDL_GPUBufferBinding ibo_binding {};
    ibo_binding.buffer = s_ibo->handle();
    ibo_binding.offset = 0;

    Uniforms uniforms;
    uniforms.mvp = s_camera.view_projection();

    SDL_BindGPUVertexBuffers(render_pass, 0, &vbo_binding, 1);
    SDL_BindGPUIndexBuffer(render_pass, &ibo_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    SDL_PushGPUVertexUniformData(command_buffer, 0, &uniforms, sizeof(uniforms));

    SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_binding, 1);

    SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);

    SDL_EndGPURenderPass(render_pass);
}
