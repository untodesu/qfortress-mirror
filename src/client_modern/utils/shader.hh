#ifndef CLIENT_MODERN_UTILS_SHADER_HH
#define CLIENT_MODERN_UTILS_SHADER_HH
#pragma once

namespace utils
{
SDL_GPUShader* create_shader(const void* bytecode, std::size_t bytecode_size, SDL_GPUShaderStage stage, std::uint32_t samplers = 0,
    std::uint32_t uniform_buffers = 0, std::uint32_t storage_buffers = 0, std::uint32_t storage_textures = 0);
} // namespace utils

#endif
