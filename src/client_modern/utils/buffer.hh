#ifndef CLIENT_MODERN_UTILS_BUFFER_HH
#define CLIENT_MODERN_UTILS_BUFFER_HH
#pragma once

namespace utils
{
void buffer_upload_wait(SDL_GPUBuffer* buffer, const void* data, std::size_t size, std::size_t offset = 0);
} // namespace utils

#endif
