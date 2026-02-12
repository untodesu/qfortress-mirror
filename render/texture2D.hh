#ifndef RENDER_TEXTURE2D_HH
#define RENDER_TEXTURE2D_HH
#pragma once

constexpr static std::uint32_t RESFLAG_TEX2D_FLIP = 1 << 8; ///< Flip vertically when loading
constexpr static std::uint32_t RESFLAG_TEX2D_GRAY = 1 << 9; ///< If set, the pixel data is 8-bit grayscale, otherwise it's RGBA8888

struct SDL_GPUTexture;

struct Texture2D final {
    static void register_resource(void);

    int width;
    int height;
    int channels;

    union {
        std::uintptr_t compat;
        SDL_GPUTexture* modern;
        ImTextureID imgui;
    };
};

#endif
