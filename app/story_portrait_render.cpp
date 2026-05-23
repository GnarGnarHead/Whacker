#include "story_portrait_render.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <utility>
#include <vector>

#include "png_rgba_loader.hpp"
#include "rgba_texture.hpp"
#include "runtime_asset_path.hpp"

namespace whacker::app {

namespace {

constexpr std::uint8_t kPortraitAlphaBoundsThreshold = 12;
constexpr std::size_t kPortraitCount = static_cast<std::size_t>(StoryPortraitId::Champion1967Contender) + 1u;

struct PortraitRaster {
    bool load_attempted = false;
    bool loaded = false;
    bool upload_attempted = false;
    int width = 0;
    int height = 0;
    RgbaTexture texture {};
    std::vector<std::uint8_t> rgba {};
};

std::array<PortraitRaster, kPortraitCount>& portrait_cache() {
    static std::array<PortraitRaster, kPortraitCount> cache {};
    return cache;
}

std::size_t portrait_index(const StoryPortraitId portrait_id) {
    const std::size_t index = static_cast<std::size_t>(portrait_id);
    return std::min(index, kPortraitCount - 1u);
}

std::filesystem::path portrait_path_for_id(const StoryPortraitId portrait_id) {
    const char* filename = story_portrait_asset_filename(portrait_id);
    if (filename == nullptr) {
        return {};
    }
    return runtime_asset_path(std::filesystem::path("story") / "characters" / "art" / filename);
}

struct AlphaBounds {
    int min_x = 0;
    int min_y = 0;
    int max_x = 0;
    int max_y = 0;
    bool valid = false;
};

AlphaBounds compute_alpha_bounds(
    const std::vector<std::uint8_t>& rgba,
    const int width,
    const int height,
    const std::uint8_t min_alpha) {
    AlphaBounds bounds {};
    if (width <= 0 || height <= 0 || rgba.size() != static_cast<std::size_t>(width * height * 4)) {
        return bounds;
    }

    bounds.min_x = width - 1;
    bounds.min_y = height - 1;
    bounds.max_x = 0;
    bounds.max_y = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::size_t alpha_offset =
                (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * 4u +
                3u;
            if (rgba[alpha_offset] < min_alpha) {
                continue;
            }
            bounds.valid = true;
            bounds.min_x = std::min(bounds.min_x, x);
            bounds.min_y = std::min(bounds.min_y, y);
            bounds.max_x = std::max(bounds.max_x, x);
            bounds.max_y = std::max(bounds.max_y, y);
        }
    }
    return bounds;
}

std::vector<std::uint8_t> crop_rgba_to_bounds(
    const std::vector<std::uint8_t>& src_rgba,
    const int src_w,
    const int src_h,
    const AlphaBounds& bounds,
    int& out_w,
    int& out_h) {
    out_w = 0;
    out_h = 0;
    if (!bounds.valid || src_w <= 0 || src_h <= 0 || src_rgba.size() != static_cast<std::size_t>(src_w * src_h * 4)) {
        return {};
    }

    const int min_x = std::clamp(bounds.min_x, 0, src_w - 1);
    const int min_y = std::clamp(bounds.min_y, 0, src_h - 1);
    const int max_x = std::clamp(bounds.max_x, min_x, src_w - 1);
    const int max_y = std::clamp(bounds.max_y, min_y, src_h - 1);
    out_w = max_x - min_x + 1;
    out_h = max_y - min_y + 1;
    if (out_w <= 0 || out_h <= 0) {
        out_w = 0;
        out_h = 0;
        return {};
    }

    std::vector<std::uint8_t> dst(static_cast<std::size_t>(out_w * out_h * 4), 0);
    for (int y = 0; y < out_h; ++y) {
        for (int x = 0; x < out_w; ++x) {
            const std::size_t src_offset =
                (static_cast<std::size_t>(y + min_y) * static_cast<std::size_t>(src_w) +
                 static_cast<std::size_t>(x + min_x)) *
                4u;
            const std::size_t dst_offset =
                (static_cast<std::size_t>(y) * static_cast<std::size_t>(out_w) + static_cast<std::size_t>(x)) * 4u;
            dst[dst_offset] = src_rgba[src_offset];
            dst[dst_offset + 1] = src_rgba[src_offset + 1];
            dst[dst_offset + 2] = src_rgba[src_offset + 2];
            dst[dst_offset + 3] = src_rgba[src_offset + 3];
        }
    }
    return dst;
}

std::array<std::uint8_t, 3> placeholder_tint_for_portrait(const StoryPortraitId portrait_id) {
    const std::size_t id = static_cast<std::size_t>(portrait_id);
    const std::array<std::uint8_t, 6> tints_r {68, 76, 90, 70, 58, 82};
    const std::array<std::uint8_t, 6> tints_g {100, 92, 84, 112, 96, 76};
    const std::array<std::uint8_t, 6> tints_b {156, 142, 168, 134, 148, 160};
    const std::size_t idx = id % tints_r.size();
    return {tints_r[idx], tints_g[idx], tints_b[idx]};
}

bool build_placeholder_portrait_rgba(
    const StoryPortraitId portrait_id,
    int& out_width,
    int& out_height,
    std::vector<std::uint8_t>& out_rgba) {
    constexpr int kSize = 64;
    out_width = kSize;
    out_height = kSize;
    out_rgba.assign(static_cast<std::size_t>(kSize * kSize * 4), 0);

    const std::array<std::uint8_t, 3> tint = placeholder_tint_for_portrait(portrait_id);
    constexpr std::uint8_t kSkinR = 236;
    constexpr std::uint8_t kSkinG = 184;
    constexpr std::uint8_t kSkinB = 128;
    constexpr std::uint8_t kPaddleR = 220;
    constexpr std::uint8_t kPaddleG = 46;
    constexpr std::uint8_t kPaddleB = 52;

    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            const float nx = (static_cast<float>(x) + 0.5f) / static_cast<float>(kSize);
            const float ny = (static_cast<float>(y) + 0.5f) / static_cast<float>(kSize);
            const float dx_head = (nx - 0.50f) / 0.17f;
            const float dy_head = (ny - 0.31f) / 0.19f;
            const bool in_head = (dx_head * dx_head + dy_head * dy_head) <= 1.0f;
            const bool in_neck = ny >= 0.44f && ny <= 0.50f && std::abs(nx - 0.50f) <= 0.08f;
            const float body_half = std::clamp(0.18f + (ny - 0.48f) * 0.46f, 0.18f, 0.36f);
            const bool in_body = ny >= 0.48f && ny <= 1.0f && std::abs(nx - 0.50f) <= body_half;
            const float dx_paddle = (nx - 0.30f) / 0.16f;
            const float dy_paddle = (ny - 0.70f) / 0.13f;
            const bool in_paddle = (dx_paddle * dx_paddle + dy_paddle * dy_paddle) <= 1.0f;

            if (!in_head && !in_neck && !in_body && !in_paddle) {
                continue;
            }

            std::uint8_t r = tint[0];
            std::uint8_t g = tint[1];
            std::uint8_t b = tint[2];
            if (in_head || in_neck) {
                r = kSkinR;
                g = kSkinG;
                b = kSkinB;
            } else if (in_paddle) {
                r = kPaddleR;
                g = kPaddleG;
                b = kPaddleB;
            } else {
                const float shade = std::clamp(1.04f - ((ny - 0.48f) * 0.52f), 0.70f, 1.08f);
                r = static_cast<std::uint8_t>(std::clamp(shade * static_cast<float>(r), 0.0f, 255.0f));
                g = static_cast<std::uint8_t>(std::clamp(shade * static_cast<float>(g), 0.0f, 255.0f));
                b = static_cast<std::uint8_t>(std::clamp(shade * static_cast<float>(b), 0.0f, 255.0f));
            }

            const std::size_t offset =
                (static_cast<std::size_t>(y) * static_cast<std::size_t>(kSize) + static_cast<std::size_t>(x)) * 4u;
            out_rgba[offset] = r;
            out_rgba[offset + 1] = g;
            out_rgba[offset + 2] = b;
            out_rgba[offset + 3] = 255u;
        }
    }
    return true;
}

bool try_load_portrait_raster(const StoryPortraitId portrait_id, PortraitRaster& out_raster) {
    out_raster.loaded = false;
    out_raster.upload_attempted = false;
    out_raster.width = 0;
    out_raster.height = 0;
    out_raster.texture = RgbaTexture {};
    out_raster.rgba.clear();

    const std::filesystem::path asset_path = portrait_path_for_id(portrait_id);
    if (asset_path.empty()) {
        return false;
    }

    PngRgbaImage src_image {};
    if (!load_png_rgba_image(asset_path, src_image, "story portrait")) {
        int placeholder_w = 0;
        int placeholder_h = 0;
        std::vector<std::uint8_t> placeholder_rgba {};
        if (!build_placeholder_portrait_rgba(portrait_id, placeholder_w, placeholder_h, placeholder_rgba)) {
            return false;
        }
        out_raster.width = placeholder_w;
        out_raster.height = placeholder_h;
        out_raster.rgba = std::move(placeholder_rgba);
        out_raster.loaded = true;
        return true;
    }

    int crop_w = src_image.width;
    int crop_h = src_image.height;
    std::vector<std::uint8_t> cropped_rgba = src_image.rgba;
    const AlphaBounds alpha_bounds =
        compute_alpha_bounds(src_image.rgba, src_image.width, src_image.height, kPortraitAlphaBoundsThreshold);
    if (alpha_bounds.valid) {
        std::vector<std::uint8_t> candidate =
            crop_rgba_to_bounds(src_image.rgba, src_image.width, src_image.height, alpha_bounds, crop_w, crop_h);
        if (!candidate.empty() && crop_w > 0 && crop_h > 0) {
            cropped_rgba = std::move(candidate);
        } else {
            crop_w = src_image.width;
            crop_h = src_image.height;
        }
    }

    if (cropped_rgba.empty() || crop_w <= 0 || crop_h <= 0) {
        return false;
    }

    out_raster.width = crop_w;
    out_raster.height = crop_h;
    out_raster.rgba = std::move(cropped_rgba);
    out_raster.loaded = true;
    return true;
}

bool ensure_portrait_texture_uploaded(PortraitRaster& raster) {
    if (raster.texture.texture_id != 0) {
        return true;
    }
    if (raster.upload_attempted) {
        return false;
    }
    if (!raster.loaded || raster.width <= 0 || raster.height <= 0 || raster.rgba.empty()) {
        return false;
    }

    raster.upload_attempted = true;
    const RgbaTextureUploadResult result =
        upload_rgba_texture(raster.rgba.data(), raster.width, raster.height, "story portrait");
    if (!result.uploaded) {
        return false;
    }
    raster.texture = result.texture;
    // Keep GPU texture as source of truth once uploaded; drop CPU copy to bound memory.
    std::vector<std::uint8_t> {}.swap(raster.rgba);
    return raster.texture.texture_id != 0;
}

const PortraitRaster& ensure_portrait_raster(const StoryPortraitId portrait_id) {
    auto& cache = portrait_cache();
    PortraitRaster& raster = cache[portrait_index(portrait_id)];
    if (raster.load_attempted) {
        return raster;
    }

    raster.load_attempted = true;
    if (portrait_id == StoryPortraitId::None) {
        return raster;
    }

    if (try_load_portrait_raster(portrait_id, raster)) {
        return raster;
    }

#ifndef NDEBUG
    assert(false && "Story portrait asset failed to load.");
#endif
    return raster;
}

}  // namespace

bool draw_story_portrait(
    const int fb_width,
    const int fb_height,
    const StoryPortraitId portrait_id,
    const float x,
    const float y,
    const float w,
    const float h,
    const float alpha,
    const float brightness,
    const bool mirror_x) {
    if (portrait_id == StoryPortraitId::None) {
        return false;
    }

    const PortraitRaster& raster_const = ensure_portrait_raster(portrait_id);
    if (!raster_const.loaded || raster_const.width <= 0 || raster_const.height <= 0) {
        return false;
    }

    auto& cache = portrait_cache();
    PortraitRaster& raster = cache[portrait_index(portrait_id)];
    if (ensure_portrait_texture_uploaded(raster)) {
        draw_rgba_texture_quad_pixels(fb_width, fb_height, raster.texture, x, y, w, h, alpha, brightness, mirror_x);
        return true;
    }
    return false;
}

void release_story_portrait_resources() {
    auto& cache = portrait_cache();
    for (PortraitRaster& raster : cache) {
        release_rgba_texture(raster.texture);
        raster = PortraitRaster {};
    }
}

}  // namespace whacker::app
