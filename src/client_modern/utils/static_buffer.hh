#ifndef CLIENT_MODERN_UTILS_STATIC_BUFFER_HH
#define CLIENT_MODERN_UTILS_STATIC_BUFFER_HH
#pragma once

namespace utils
{
class StaticBuffer final {
public:
    explicit StaticBuffer(std::size_t capacity, SDL_GPUBufferUsageFlags usage);
    virtual ~StaticBuffer(void) noexcept;

    void upload(std::span<const std::byte> data, std::ptrdiff_t byte_offset = 0);

    template<typename T>
    void upload(std::span<const T> data, std::ptrdiff_t byte_offset = 0);

    constexpr std::size_t capacity(void) const noexcept;
    constexpr SDL_GPUBufferUsageFlags usage(void) const noexcept;
    constexpr const SDL_GPUBuffer* handle(void) const noexcept;
    constexpr SDL_GPUBuffer* handle(void) noexcept;

private:
    std::size_t m_capacity;
    SDL_GPUBufferUsageFlags m_usage;
    SDL_GPUBuffer* m_gpu_handle;
};
} // namespace utils

template<typename T>
void utils::StaticBuffer::upload(std::span<const T> data, std::ptrdiff_t byte_offset)
{
    upload(std::as_bytes(data), byte_offset);
}

constexpr std::size_t utils::StaticBuffer::capacity(void) const noexcept
{
    return m_capacity;
}

constexpr SDL_GPUBufferUsageFlags utils::StaticBuffer::usage(void) const noexcept
{
    return m_usage;
}

constexpr const SDL_GPUBuffer* utils::StaticBuffer::handle(void) const noexcept
{
    return m_gpu_handle;
}

constexpr SDL_GPUBuffer* utils::StaticBuffer::handle(void) noexcept
{
    return m_gpu_handle;
}

#endif
