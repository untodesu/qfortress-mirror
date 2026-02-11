#ifndef CLIENT_MODERN_UTILS_BUFFER_HH
#define CLIENT_MODERN_UTILS_BUFFER_HH
#pragma once

namespace utils
{
SDL_GPUBuffer* create_buffer(std::size_t size, SDL_GPUBufferUsageFlags usage);
SDL_GPUBuffer* initialize_buffer(const void* data, std::size_t size, SDL_GPUBufferUsageFlags usage);
void fill_buffer_blocking(SDL_GPUBuffer* buffer, const void* data, std::size_t size, std::size_t offset = 0);
} // namespace utils

#endif
