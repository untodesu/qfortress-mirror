#ifndef CLIENT_MODERN_UTILS_STREAM_BUFFER_HH
#define CLIENT_MODERN_UTILS_STREAM_BUFFER_HH
#pragma once

namespace utils
{
class StreamBuffer final {
public:
    explicit StreamBuffer(std::size_t size, SDL_GPUBufferUsageFlags usage, std::size_t backing = 2);
    virtual ~StreamBuffer(void) noexcept;

    SDL_GPUBuffer* upload(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data);

    template<typename T>
    SDL_GPUBuffer* upload(SDL_GPUCopyPass* copy_pass, std::span<const T> data);

    void update_late(void);

private:
    std::size_t m_size;
    std::size_t m_curframe;
    std::vector<SDL_GPUBuffer*> m_buffers;
};
} // namespace utils

template<typename T>
SDL_GPUBuffer* utils::StreamBuffer::upload(SDL_GPUCopyPass* copy_pass, std::span<const T> data)
{
    return upload(copy_pass, std::as_bytes(data));
}

#endif
