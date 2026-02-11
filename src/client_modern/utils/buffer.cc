#include "client_modern/pch.hh"

#include "client_modern/utils/buffer.hh"

#include "core/exceptions.hh"

#include "client_modern/globals.hh"

SDL_GPUBuffer* utils::create_buffer(std::size_t size, SDL_GPUBufferUsageFlags usage)
{
    assert(size);
    assert(globals::gpu_device);

    SDL_GPUBufferCreateInfo buffer_info {};
    buffer_info.size = static_cast<Uint32>(size);
    buffer_info.usage = usage;

    auto buffer = SDL_CreateGPUBuffer(globals::gpu_device, &buffer_info);
    qf::throw_if_not_fmt<std::runtime_error>(buffer, "SDL_CreateGPUBuffer failed: {}", SDL_GetError());

    return buffer;
}

SDL_GPUBuffer* utils::initialize_buffer(const void* data, std::size_t size, SDL_GPUBufferUsageFlags usage)
{
    assert(data);
    assert(size);
    assert(globals::gpu_device);

    auto buffer = utils::create_buffer(size, usage);

    utils::fill_buffer_blocking(buffer, data, size);

    return buffer;
}

void utils::fill_buffer_blocking(SDL_GPUBuffer* buffer, const void* data, std::size_t size, std::size_t offset)
{
    assert(buffer);
    assert(data);
    assert(size);
    assert(globals::gpu_device);

    SDL_GPUTransferBufferCreateInfo transfer_info {};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = static_cast<Uint32>(size);

    auto transfer_buffer = SDL_CreateGPUTransferBuffer(globals::gpu_device, &transfer_info);
    qf::throw_if_not_fmt<std::runtime_error>(transfer_buffer, "SDL_CreateGPUTransferBuffer failed: {}", SDL_GetError());

    auto map_ptr = SDL_MapGPUTransferBuffer(globals::gpu_device, transfer_buffer, false);
    qf::throw_if_not_fmt<std::runtime_error>(map_ptr, "SDL_MapGPUTransferBuffer failed: {}", SDL_GetError());

    std::memcpy(map_ptr, data, size);

    SDL_UnmapGPUTransferBuffer(globals::gpu_device, transfer_buffer);

    auto command_buffer = SDL_AcquireGPUCommandBuffer(globals::gpu_device);
    qf::throw_if_not_fmt<std::runtime_error>(command_buffer, "SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());

    auto copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    qf::throw_if_not<std::runtime_error>(copy_pass, "SDL_BeginGPUCopyPass returned null");

    SDL_GPUTransferBufferLocation source {};
    source.transfer_buffer = transfer_buffer;
    source.offset = 0;

    SDL_GPUBufferRegion destination {};
    destination.buffer = buffer;
    destination.offset = static_cast<Uint32>(offset);
    destination.size = static_cast<Uint32>(size);

    SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);

    SDL_EndGPUCopyPass(copy_pass);

    // Submit and wait; this is the blocking part
    // that shouldn't be used on a per-frame basis
    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(globals::gpu_device);

    SDL_ReleaseGPUTransferBuffer(globals::gpu_device, transfer_buffer);
}
