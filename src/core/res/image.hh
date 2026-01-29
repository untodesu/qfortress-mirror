#ifndef CORE_RES_IMAGE_HH
#define CORE_RES_IMAGE_HH
#pragma once

constexpr static unsigned RESFLAG_IMG_FLIP = 1 << 8; ///< Flip image vertically on load
constexpr static unsigned RESFLAG_IMG_GRAY = 1 << 9; ///< If set, the pixel data is 8-bit grayscale, otherwise it's RGBA8888

struct Image final {
    static void register_resource(void);

    int width;
    int height;
    int channels;
    stbi_uc* pixels;
};

#endif
