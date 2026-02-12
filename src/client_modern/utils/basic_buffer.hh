#ifndef CLIENT_MODERN_UTILS_BASIC_BUFFER_HH
#define CLIENT_MODERN_UTILS_BASIC_BUFFER_HH
#pragma once

namespace utils
{
class BasicBuffer final {
public:
    explicit BasicBuffer(std::size_t size, SDL_GPUBufferUsageFlags usage);
    virtual ~BasicBuffer(void) noexcept;

    void upload_wait(std::span<const std::byte> data, std::ptrdiff_t byte_offset = 0);

    template<typename T>
    void upload_wait(std::span<const T> data, std::ptrdiff_t byte_offset = 0);

    constexpr const SDL_GPUBuffer* handle(void) const noexcept;
    constexpr SDL_GPUBuffer* handle(void) noexcept;

private:
    SDL_GPUBuffer* m_handle;
};
} // namespace utils

template<typename T>
void utils::BasicBuffer::upload_wait(std::span<const T> data, std::ptrdiff_t byte_offset)
{
    upload_wait(std::as_bytes(data), byte_offset);
}

constexpr const SDL_GPUBuffer* utils::BasicBuffer::handle(void) const noexcept
{
    return m_handle;
}

constexpr SDL_GPUBuffer* utils::BasicBuffer::handle(void) noexcept
{
    return m_handle;
}

#endif
