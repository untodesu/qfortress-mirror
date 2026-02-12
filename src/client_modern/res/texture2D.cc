#include "client_modern/pch.hh"

#include "client/res/texture2D.hh"

#include "core/exceptions.hh"
#include "core/res/image.hh"
#include "core/res/resource.hh"

#include "client_modern/globals.hh"

template<std::uint32_t TextureBit, std::uint32_t ImageBit>
std::uint32_t build_image_flags(std::uint32_t image_flags, std::uint32_t texture_flags)
{
    if(texture_flags & TextureBit)
        return image_flags | ImageBit;
    return image_flags;
}

static const void* texture2D_load_fn_modern(const char* name, std::uint32_t flags)
{
    assert(name);
    assert(globals::gpu_device);

    std::uint32_t image_flags = 0;
    image_flags = build_image_flags<RESFLAG_TEX2D_FLIP, RESFLAG_IMG_FLIP>(image_flags, flags);
    image_flags = build_image_flags<RESFLAG_TEX2D_GRAY, RESFLAG_IMG_GRAY>(image_flags, flags);

    SDL_GPUTextureFormat image_format;
    std::size_t pixel_size_bytes;

    if(flags & RESFLAG_TEX2D_GRAY) {
        image_format = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
        pixel_size_bytes = 1;
    }
    else {
        image_format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        pixel_size_bytes = 4;
    }

    auto image = res::load<Image>(name, image_flags);

    if(image == nullptr) {
        LOG_WARNING("{}: image load failed", name);
        return nullptr;
    }

    SDL_GPUTextureCreateInfo texture_info {};
    texture_info.type = SDL_GPU_TEXTURETYPE_2D;
    texture_info.format = image_format;
    texture_info.width = image->width;
    texture_info.height = image->height;
    texture_info.layer_count_or_depth = 1;
    texture_info.num_levels = 1;
    texture_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    auto gpu_handle = SDL_CreateGPUTexture(globals::gpu_device, &texture_info);

    if(gpu_handle == nullptr) {
        LOG_WARNING("{}: failed to create a GPU gpu_handle: {}", name, SDL_GetError());
        return nullptr;
    }

    SDL_GPUTransferBufferCreateInfo transfer_info {};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = static_cast<Uint32>(pixel_size_bytes * image->width * image->height);

    auto transfer_buffer = SDL_CreateGPUTransferBuffer(globals::gpu_device, &transfer_info);

    if(transfer_buffer == nullptr) {
        LOG_WARNING("{}: failed to create a GPU transfer buffer: {}", name, SDL_GetError());
        SDL_ReleaseGPUTexture(globals::gpu_device, gpu_handle);
        return nullptr;
    }

    auto transfer_ptr = SDL_MapGPUTransferBuffer(globals::gpu_device, transfer_buffer, false);

    if(transfer_ptr == nullptr) {
        LOG_WARNING("{}: failed to map a GPU transfer buffer: {}", name, SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(globals::gpu_device, transfer_buffer);
        SDL_ReleaseGPUTexture(globals::gpu_device, gpu_handle);
        return nullptr;
    }

    std::memcpy(transfer_ptr, image->pixels, transfer_info.size);

    auto command_buffer = SDL_AcquireGPUCommandBuffer(globals::gpu_device);

    if(command_buffer == nullptr) {
        LOG_WARNING("{}: failed to acquire a GPU command buffer: {}", name, SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(globals::gpu_device, transfer_buffer);
        SDL_ReleaseGPUTexture(globals::gpu_device, gpu_handle);
        return nullptr;
    }

    auto copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    qf::throw_if_not<std::runtime_error>(copy_pass, "SDL_BeginGPUCopyPass returned nullptr");

    SDL_GPUTextureTransferInfo source {};
    source.transfer_buffer = transfer_buffer;
    source.offset = 0;

    SDL_GPUTextureRegion destination {};
    destination.texture = gpu_handle;
    destination.w = image->width;
    destination.h = image->height;
    destination.d = 1;

    SDL_UploadToGPUTexture(copy_pass, &source, &destination, false);

    SDL_EndGPUCopyPass(copy_pass);

    // Submit and wait; this is the blocking part
    // that shouldn't be used on a per-frame basis
    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(globals::gpu_device);

    SDL_ReleaseGPUTransferBuffer(globals::gpu_device, transfer_buffer);

    auto texture = new Texture2D;
    texture->width = image->width;
    texture->height = image->height;
    texture->channels = image->channels;
    texture->modern = gpu_handle;

    return texture;
}

static void texture2D_free_fn_modern(const void* resource)
{
    assert(resource);

    auto texture = reinterpret_cast<const Texture2D*>(resource);
    SDL_ReleaseGPUTexture(globals::gpu_device, texture->modern);

    delete texture;
}

void Texture2D::register_resource(void)
{
    res::register_loader<Texture2D>(&texture2D_load_fn_modern, &texture2D_free_fn_modern);
}
