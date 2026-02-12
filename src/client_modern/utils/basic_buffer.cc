#include "client_modern/pch.hh"

#include "client_modern/utils/basic_buffer.hh"

#include "core/exceptions.hh"

#include "client_modern/globals.hh"

utils::BasicBuffer::BasicBuffer(std::size_t size, SDL_GPUBufferUsageFlags usage)
{
    assert(size);
    assert(usage);

    assert(globals::gpu_device);

    SDL_GPUBufferCreateInfo buffer_info {};
    buffer_info.size = static_cast<Uint32>(size);
    buffer_info.usage = usage;

    m_handle = SDL_CreateGPUBuffer(globals::gpu_device, &buffer_info);
    qf::throw_if_not_fmt<std::runtime_error>(m_handle, "failed to create a GPU buffer: {}", SDL_GetError());
}

utils::BasicBuffer::~BasicBuffer(void) noexcept
{
    SDL_ReleaseGPUBuffer(globals::gpu_device, m_handle);
}

void utils::BasicBuffer::upload_wait(std::span<const std::byte> data, std::ptrdiff_t byte_offset)
{
    assert(data.size_bytes());
    assert(byte_offset >= 0);

    assert(globals::gpu_device);

    SDL_GPUTransferBufferCreateInfo transfer_info {};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = static_cast<Uint32>(data.size_bytes());

    auto transfer_buffer = SDL_CreateGPUTransferBuffer(globals::gpu_device, &transfer_info);
    qf::throw_if_not_fmt<std::runtime_error>(transfer_buffer, "failed to create a GPU transfer buffer: {}", SDL_GetError());

    auto transfer_ptr = SDL_MapGPUTransferBuffer(globals::gpu_device, transfer_buffer, false);
    qf::throw_if_not_fmt<std::runtime_error>(transfer_ptr, "failed to map a GPU transfer buffer: {}", SDL_GetError());

    std::memcpy(transfer_ptr, data.data(), data.size_bytes());

    SDL_UnmapGPUTransferBuffer(globals::gpu_device, transfer_buffer);

    auto command_buffer = SDL_AcquireGPUCommandBuffer(globals::gpu_device);
    qf::throw_if_not_fmt<std::runtime_error>(command_buffer, "failed to acquire a GPU command buffer: {}", SDL_GetError());

    auto copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    qf::throw_if_not<std::runtime_error>(copy_pass, "SDL_BeginGPUCopyPass returned nullptr");

    SDL_GPUTransferBufferLocation source {};
    source.transfer_buffer = transfer_buffer;
    source.offset = 0;

    SDL_GPUBufferRegion destination {};
    destination.buffer = m_handle;
    destination.offset = static_cast<Uint32>(byte_offset);
    destination.size = static_cast<Uint32>(data.size_bytes());

    SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);

    SDL_EndGPUCopyPass(copy_pass);

    // Submit and wait; this is the blocking part
    // that shouldn't be used on a per-frame basis
    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(globals::gpu_device);

    SDL_ReleaseGPUTransferBuffer(globals::gpu_device, transfer_buffer);
}
