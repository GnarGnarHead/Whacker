#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace whacker::app {

struct PngRgbaImage {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba {};
};

bool load_png_rgba_image(const std::filesystem::path& asset_path, PngRgbaImage& out_image, const char* debug_label);

}  // namespace whacker::app
