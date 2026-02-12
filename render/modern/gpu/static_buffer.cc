#include "render/modern/pch.hh"

#include "render/modern/gpu/static_buffer.hh"

#include "core/exceptions.hh"

#include "render/modern/globals.hh"

gpu::StaticBuffer::StaticBuffer(std::size_t capacity, SDL_GPUBufferUsageFlags usage) : m_capacity(capacity), m_usage(usage)
{
    assert(capacity);
    assert(usage);

    assert(globals::gpu_device);

    SDL_GPUBufferCreateInfo buffer_info {};
    buffer_info.size = static_cast<Uint32>(m_capacity);
    buffer_info.usage = m_usage;

    m_gpu_handle = SDL_CreateGPUBuffer(globals::gpu_device, &buffer_info);
    qf::throw_if_not_fmt<std::runtime_error>(m_gpu_handle, "failed to create a GPU buffer: {}", SDL_GetError());
}

gpu::StaticBuffer::~StaticBuffer(void) noexcept
{
    SDL_ReleaseGPUBuffer(globals::gpu_device, m_gpu_handle);
}

void gpu::StaticBuffer::upload(std::span<const std::byte> data, std::ptrdiff_t offset)
{
    assert(data.size_bytes());
    assert(offset >= 0 && offset + data.size_bytes() <= m_capacity);

    assert(globals::gpu_device);
    assert(m_gpu_handle);

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
    destination.buffer = m_gpu_handle;
    destination.size = static_cast<Uint32>(data.size_bytes());
    destination.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);

    SDL_EndGPUCopyPass(copy_pass);

    // Submit and wait; this is the blocking part
    // that shouldn't be used on a per-frame basis
    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(globals::gpu_device);

    SDL_ReleaseGPUTransferBuffer(globals::gpu_device, transfer_buffer);
}
