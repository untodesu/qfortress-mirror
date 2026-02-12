#include "render/modern/pch.hh"

#include "render/modern/gpu/stream_buffer.hh"

#include "core/exceptions.hh"

#include "render/modern/globals.hh"

gpu::StreamBuffer::StreamBuffer(std::size_t capacity, SDL_GPUBufferUsageFlags usage, std::size_t backing)
    : m_capacity(capacity), m_current_frame(0), m_usage(usage)
{
    assert(capacity);
    assert(usage);
    assert(backing);

    assert(globals::gpu_device);

    m_buffers.resize(backing, nullptr);

    SDL_GPUBufferCreateInfo buffer_info {};
    buffer_info.size = static_cast<Uint32>(m_capacity);
    buffer_info.usage = usage;

    for(std::size_t i = 0; i < m_buffers.size(); ++i) {
        m_buffers[i] = SDL_CreateGPUBuffer(globals::gpu_device, &buffer_info);
        qf::throw_if_not_fmt<std::runtime_error>(m_buffers[i], "failed to create a GPU buffer: {}", SDL_GetError());
    }
}

gpu::StreamBuffer::~StreamBuffer(void)
{
    for(auto buffer : m_buffers) {
        SDL_ReleaseGPUBuffer(globals::gpu_device, buffer);
    }
}

SDL_GPUBuffer* gpu::StreamBuffer::upload(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data)
{
    assert(copy_pass);
    assert(data.size_bytes());

    assert(globals::gpu_device);

    SDL_GPUTransferBufferCreateInfo transfer_info {};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = static_cast<Uint32>(data.size_bytes());

    auto transfer_buffer = SDL_CreateGPUTransferBuffer(globals::gpu_device, &transfer_info);
    qf::throw_if_not_fmt<std::runtime_error>(transfer_buffer, "failed to create a GPU transfer buffer: {}", SDL_GetError());

    auto transfer_ptr = SDL_MapGPUTransferBuffer(globals::gpu_device, transfer_buffer, false);

    std::memcpy(transfer_ptr, data.data(), data.size_bytes());

    SDL_UnmapGPUTransferBuffer(globals::gpu_device, transfer_buffer);

    auto buffer = m_buffers[m_current_frame];

    SDL_GPUTransferBufferLocation source {};
    source.transfer_buffer = transfer_buffer;
    source.offset = 0;

    SDL_GPUBufferRegion destination {};
    destination.buffer = buffer;
    destination.size = static_cast<Uint32>(data.size_bytes());
    destination.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);

    SDL_ReleaseGPUTransferBuffer(globals::gpu_device, transfer_buffer);

    return buffer;
}

void gpu::StreamBuffer::update_late(void)
{
    m_current_frame += 1;
    m_current_frame %= m_buffers.size();
}
