#ifndef RENDER_MODERN_GPU_STREAM_BUFFER_HH
#define RENDER_MODERN_GPU_STREAM_BUFFER_HH
#pragma once

namespace gpu
{
class StreamBuffer final {
public:
    explicit StreamBuffer(std::size_t capacity, SDL_GPUBufferUsageFlags usage, std::size_t backing = 2);
    virtual ~StreamBuffer(void) noexcept;

    SDL_GPUBuffer* upload(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data);

    template<typename T>
    SDL_GPUBuffer* upload(SDL_GPUCopyPass* copy_pass, std::span<const T> data);

    void update_late(void);

    constexpr std::size_t backing(void) const noexcept;
    constexpr std::size_t capacity(void) const noexcept;
    constexpr SDL_GPUBufferUsageFlags usage(void) const noexcept;

private:
    std::size_t m_capacity;
    std::size_t m_current_frame;
    SDL_GPUBufferUsageFlags m_usage;
    std::vector<SDL_GPUBuffer*> m_buffers;
};
} // namespace gpu

template<typename T>
SDL_GPUBuffer* gpu::StreamBuffer::upload(SDL_GPUCopyPass* copy_pass, std::span<const T> data)
{
    return upload(copy_pass, std::as_bytes(data));
}

constexpr std::size_t gpu::StreamBuffer::backing(void) const noexcept
{
    return m_buffers.size();
}

constexpr std::size_t gpu::StreamBuffer::capacity(void) const noexcept
{
    return m_capacity;
}

constexpr SDL_GPUBufferUsageFlags gpu::StreamBuffer::usage(void) const noexcept
{
    return m_usage;
}

#endif
