#include "client_modern/pch.hh"

#include "client_modern/utils/shader.hh"

#include "core/exceptions.hh"

#include "client_modern/globals.hh"

SDL_GPUShader* utils::create_shader(const void* bytecode, std::size_t bytecode_size, SDL_GPUShaderStage stage, std::uint32_t samplers,
    std::uint32_t uniform_buffers, std::uint32_t storage_buffers, std::uint32_t storage_textures)
{
    assert(bytecode);
    assert(bytecode_size);
    assert(globals::gpu_device);

    SDL_GPUShaderCreateInfo create_info {};
    create_info.code_size = bytecode_size;
    create_info.code = reinterpret_cast<const Uint8*>(bytecode);
    create_info.entrypoint = "main"; // Why would anyone use anything else?
    create_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    create_info.stage = stage;
    create_info.num_samplers = samplers;
    create_info.num_storage_textures = storage_textures;
    create_info.num_storage_buffers = storage_buffers;
    create_info.num_uniform_buffers = uniform_buffers;

    auto shader = SDL_CreateGPUShader(globals::gpu_device, &create_info);
    qf::throw_if_not_fmt<std::runtime_error>(shader, "SDL_CreateGPUShader failed: {}", SDL_GetError());

    return shader;
}
