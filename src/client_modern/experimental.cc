#include "client_modern/pch.hh"

#include "client_modern/experimental.hh"

#include "core/exceptions.hh"

#include "client_modern/globals.hh"
#include "client_modern/utils/buffer.hh"

extern const std::uint8_t spirv_experimental_vert[];
extern const std::size_t spirv_experimental_vert_size;

extern const std::uint8_t spirv_experimental_frag[];
extern const std::size_t spirv_experimental_frag_size;

static SDL_GPUGraphicsPipeline* s_pipeline;
static std::unique_ptr<utils::Buffer> s_vbo;
static std::unique_ptr<utils::Buffer> s_ibo;

struct Vertex final {
    Eigen::Vector3f position;
    Eigen::Vector2f texcoord;
};

void experimental::init(void)
{
    // empty
}

void experimental::init_late(void)
{
    SDL_GPUShaderCreateInfo vert_info {};
    vert_info.code_size = spirv_experimental_vert_size;
    vert_info.code = reinterpret_cast<const Uint8*>(spirv_experimental_vert);
    vert_info.entrypoint = "main";
    vert_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    vert_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;

    SDL_GPUShaderCreateInfo frag_info {};
    frag_info.code_size = spirv_experimental_frag_size;
    frag_info.code = reinterpret_cast<const Uint8*>(spirv_experimental_frag);
    frag_info.entrypoint = "main";
    frag_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    frag_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;

    auto vert = SDL_CreateGPUShader(globals::gpu_device, &vert_info);
    qf::throw_if_not_fmt<std::runtime_error>(vert, "SDL_CreateGPUShader (vert) failed: {}", SDL_GetError());

    auto frag = SDL_CreateGPUShader(globals::gpu_device, &frag_info);
    qf::throw_if_not_fmt<std::runtime_error>(frag, "SDL_CreateGPUShader (frag) failed: {}", SDL_GetError());

    std::vector<Vertex> vertices;
    vertices.push_back({ Eigen::Vector3f(-0.5f, -0.5f, 0.0f), Eigen::Vector2f(0.0f, 0.0f) });
    vertices.push_back({ Eigen::Vector3f(-0.5f, +0.5f, 0.0f), Eigen::Vector2f(0.0f, 1.0f) });
    vertices.push_back({ Eigen::Vector3f(+0.5f, +0.5f, 0.0f), Eigen::Vector2f(1.0f, 1.0f) });
    vertices.push_back({ Eigen::Vector3f(+0.5f, -0.5f, 0.0f), Eigen::Vector2f(1.0f, 0.0f) });

    std::vector<std::uint32_t> indices;
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);

    s_vbo = std::make_unique<utils::Buffer>(sizeof(Vertex) * vertices.size(), SDL_GPU_BUFFERUSAGE_VERTEX);
    s_vbo->upload_wait<Vertex>(vertices);

    s_ibo = std::make_unique<utils::Buffer>(sizeof(std::uint32_t) * indices.size(), SDL_GPU_BUFFERUSAGE_INDEX);
    s_ibo->upload_wait<std::uint32_t>(indices);

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
    pipeline_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
    pipeline_info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    s_pipeline = SDL_CreateGPUGraphicsPipeline(globals::gpu_device, &pipeline_info);
    qf::throw_if_not_fmt<std::runtime_error>(s_pipeline, "SDL_CreateGPUGraphicsPipeline failed: {}", SDL_GetError());

    SDL_ReleaseGPUShader(globals::gpu_device, frag);
    SDL_ReleaseGPUShader(globals::gpu_device, vert);
}

void experimental::shutdown(void)
{
    s_ibo.reset();
    s_vbo.reset();

    SDL_ReleaseGPUGraphicsPipeline(globals::gpu_device, s_pipeline);
}

void experimental::render(SDL_GPURenderPass* render_pass)
{
    assert(render_pass);

    SDL_BindGPUGraphicsPipeline(render_pass, s_pipeline);

    SDL_GPUBufferBinding vbo_binding {};
    vbo_binding.buffer = s_vbo->handle();
    vbo_binding.offset = 0;

    SDL_GPUBufferBinding ibo_binding {};
    ibo_binding.buffer = s_ibo->handle();
    ibo_binding.offset = 0;

    SDL_BindGPUVertexBuffers(render_pass, 0, &vbo_binding, 1);
    SDL_BindGPUIndexBuffer(render_pass, &ibo_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);
}
