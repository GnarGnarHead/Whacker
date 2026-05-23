#include "png_rgba_loader.hpp"

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>

#if defined(WHACKER_HAS_PNG)
#include <png.h>
#endif

namespace whacker::app {

namespace {

const char* safe_debug_label(const char* debug_label) {
    return debug_label != nullptr && debug_label[0] != '\0' ? debug_label : "PNG";
}

void reset_image(PngRgbaImage& image) {
    image.width = 0;
    image.height = 0;
    image.rgba.clear();
}

}  // namespace

bool load_png_rgba_image(const std::filesystem::path& asset_path, PngRgbaImage& out_image, const char* debug_label) {
    reset_image(out_image);

#if defined(WHACKER_HAS_PNG)
    const std::string asset_path_string = asset_path.string();
    errno = 0;
    std::FILE* file = std::fopen(asset_path_string.c_str(), "rb");
    if (file == nullptr) {
        const int open_errno = errno;
        std::fprintf(
            stderr,
            "PNG load failed for %s: could not open %s (%s)\n",
            safe_debug_label(debug_label),
            asset_path_string.c_str(),
            std::strerror(open_errno));
        return false;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr) {
        std::fclose(file);
        std::fprintf(
            stderr,
            "PNG load failed for %s: png_create_read_struct failed for %s\n",
            safe_debug_label(debug_label),
            asset_path_string.c_str());
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (info == nullptr) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        std::fclose(file);
        std::fprintf(
            stderr,
            "PNG load failed for %s: png_create_info_struct failed for %s\n",
            safe_debug_label(debug_label),
            asset_path_string.c_str());
        return false;
    }

    if (setjmp(png_jmpbuf(png)) != 0) {
        png_destroy_read_struct(&png, &info, nullptr);
        std::fclose(file);
        std::fprintf(
            stderr,
            "PNG load failed for %s: libpng rejected %s\n",
            safe_debug_label(debug_label),
            asset_path_string.c_str());
        return false;
    }

    png_init_io(png, file);
    png_read_info(png, info);

    const png_uint_32 width = png_get_image_width(png, info);
    const png_uint_32 height = png_get_image_height(png, info);
    const int bit_depth = png_get_bit_depth(png, info);
    const int color_type = png_get_color_type(png, info);

    if (bit_depth == 16) {
        png_set_strip_16(png);
    }
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    if (png_get_valid(png, info, PNG_INFO_tRNS) != 0) {
        png_set_tRNS_to_alpha(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    }

    png_read_update_info(png, info);

    if (width == 0 || height == 0 || width > 8192u || height > 8192u) {
        png_destroy_read_struct(&png, &info, nullptr);
        std::fclose(file);
        std::fprintf(
            stderr,
            "PNG load failed for %s: unsupported image size %ux%u at %s\n",
            safe_debug_label(debug_label),
            static_cast<unsigned>(width),
            static_cast<unsigned>(height),
            asset_path_string.c_str());
        return false;
    }

    out_image.width = static_cast<int>(width);
    out_image.height = static_cast<int>(height);
    out_image.rgba.assign(static_cast<std::size_t>(out_image.width * out_image.height * 4), 0);

    std::vector<png_bytep> row_ptrs(static_cast<std::size_t>(out_image.height));
    for (int y = 0; y < out_image.height; ++y) {
        row_ptrs[static_cast<std::size_t>(y)] = reinterpret_cast<png_bytep>(
            out_image.rgba.data() + static_cast<std::size_t>(y * out_image.width * 4));
    }
    png_read_image(png, row_ptrs.data());
    png_read_end(png, nullptr);

    png_destroy_read_struct(&png, &info, nullptr);
    std::fclose(file);
    return true;
#else
    std::fprintf(
        stderr,
        "PNG load failed for %s: libpng support is not compiled in; requested %s\n",
        safe_debug_label(debug_label),
        asset_path.string().c_str());
    return false;
#endif
}

}  // namespace whacker::app
