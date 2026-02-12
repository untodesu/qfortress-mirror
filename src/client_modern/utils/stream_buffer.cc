#include "client_modern/pch.hh"

#include "client_modern/utils/stream_buffer.hh"

#include "core/exceptions.hh"

#include "client_modern/globals.hh"

utils::StreamBuffer::StreamBuffer(std::size_t size, SDL_GPUBufferUsageFlags usage, std::size_t backing) : m_size(size), m_curframe(0)
{
    assert(size);
    assert(backing);

    assert(globals::gpu_device);

    m_buffers.reserve(backing);

    SDL_GPUBufferCreateInfo buffer_info {};
    buffer_info.size = static_cast<Uint32>(size);
    buffer_info.usage = usage;

    for(std::size_t i = 0; i < backing; ++i) {
        auto buffer = SDL_CreateGPUBuffer(globals::gpu_device, &buffer_info);
        qf::throw_if_not_fmt<std::runtime_error>(buffer, "failed to create a GPU buffer: {}", SDL_GetError());

        m_buffers.emplace_back(std::move(buffer));
    }
}

utils::StreamBuffer::~StreamBuffer(void)
{
    for(auto buffer : m_buffers) {
        SDL_ReleaseGPUBuffer(globals::gpu_device, buffer);
    }
}

SDL_GPUBuffer* utils::StreamBuffer::upload(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data)
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

    auto device_buffer = m_buffers[m_curframe];

    SDL_GPUTransferBufferLocation source {};
    source.transfer_buffer = transfer_buffer;
    source.offset = 0;

    SDL_GPUBufferRegion destination {};
    destination.buffer = device_buffer;
    destination.size = static_cast<Uint32>(data.size_bytes());
    destination.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);

    SDL_ReleaseGPUTransferBuffer(globals::gpu_device, transfer_buffer);

    return device_buffer;
}

void utils::StreamBuffer::update_late(void)
{
    m_curframe += 1;
    m_curframe %= m_buffers.size();
}
