#ifndef CLIENT_RES_TEXTURE2D_HH
#define CLIENT_RES_TEXTURE2D_HH
#pragma once

constexpr static std::uint32_t RESFLAG_TEX2D_FLIP = 1 << 8; ///< Flip vertically when loading
constexpr static std::uint32_t RESFLAG_TEX2D_GRAY = 1 << 9; ///< If set, the pixel data is 8-bit grayscale, otherwise it's RGBA8888

struct Texture2D final {
    static void register_resource(void);

    int width;
    int height;
    int channels;     ///< 1 for R8_UNORM, 4 for R8G8B8A8_UNORM
    void* gpu_handle; ///< GLuint for compat, SDL_GPUTexture pointer for modern
};

#endif
