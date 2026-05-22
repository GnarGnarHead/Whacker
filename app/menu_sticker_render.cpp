#include "menu_sticker_render.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <GL/gl.h>

#if defined(WHACKER_HAS_PNG)
#include <png.h>
#endif

#include "pixel_font.hpp"
#include "story_pack.hpp"

#ifndef WHACKER_SOURCE_DIR
#define WHACKER_SOURCE_DIR "."
#endif

namespace whacker::app {

namespace {

constexpr int kMinVisibleStickerPixels = 16;
constexpr float kMinStickerSizePx = 24.0f;
constexpr float kMaxStickerSizePx = 260.0f;
constexpr float kInsideInsetPx = 6.0f;
constexpr float kProtectedOverlapThresholdPx = 1.0f;
constexpr float kStickerOverlapThresholdPx = 2200.0f;

struct StickerTexture {
    bool load_attempted = false;
    bool loaded = false;
    bool uploaded = false;
    int width = 0;
    int height = 0;
    GLuint texture_id = 0;
    std::vector<std::uint8_t> rgba {};
};

std::unordered_map<std::string, StickerTexture>& sticker_texture_cache() {
    static std::unordered_map<std::string, StickerTexture> cache {};
    return cache;
}

float overlap_area(const MenuStickerRect& a, const MenuStickerRect& b) {
    const float x0 = std::max(a.x, b.x);
    const float y0 = std::max(a.y, b.y);
    const float x1 = std::min(a.x + a.w, b.x + b.w);
    const float y1 = std::min(a.y + a.h, b.y + b.h);
    if (x1 <= x0 || y1 <= y0) {
        return 0.0f;
    }
    return (x1 - x0) * (y1 - y0);
}

struct ScopedStickerScissor {
    ScopedStickerScissor(const int fb_width, const int fb_height) {
        was_enabled_ = (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
        glGetIntegerv(GL_SCISSOR_BOX, previous_box_.data());
        glEnable(GL_SCISSOR_TEST);
        apply_full_pixel_scissor(fb_width, fb_height);
    }

    ~ScopedStickerScissor() {
        if (was_enabled_) {
            glEnable(GL_SCISSOR_TEST);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }
        glScissor(previous_box_[0], previous_box_[1], previous_box_[2], previous_box_[3]);
    }

    bool was_enabled_ = false;
    std::array<GLint, 4> previous_box_ {0, 0, 0, 0};
};

std::filesystem::path sticker_asset_path(const std::string_view filename) {
    return std::filesystem::path(WHACKER_SOURCE_DIR) / "story" / "art" / "Stickers" / std::string(filename);
}

#if defined(WHACKER_HAS_PNG)
bool load_png_rgba(
    const std::filesystem::path& asset_path,
    int& out_width,
    int& out_height,
    std::vector<std::uint8_t>& out_rgba) {
    out_width = 0;
    out_height = 0;
    out_rgba.clear();

    std::FILE* file = std::fopen(asset_path.string().c_str(), "rb");
    if (file == nullptr) {
        return false;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr) {
        std::fclose(file);
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (info == nullptr) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        std::fclose(file);
        return false;
    }

    if (setjmp(png_jmpbuf(png)) != 0) {
        png_destroy_read_struct(&png, &info, nullptr);
        std::fclose(file);
        return false;
    }

    png_init_io(png, file);
    png_read_info(png, info);

    png_uint_32 width = png_get_image_width(png, info);
    png_uint_32 height = png_get_image_height(png, info);
    int bit_depth = png_get_bit_depth(png, info);
    int color_type = png_get_color_type(png, info);

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
    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    }

    png_read_update_info(png, info);

    if (width == 0 || height == 0 || width > 8192u || height > 8192u) {
        png_destroy_read_struct(&png, &info, nullptr);
        std::fclose(file);
        return false;
    }

    out_width = static_cast<int>(width);
    out_height = static_cast<int>(height);
    out_rgba.assign(static_cast<std::size_t>(out_width * out_height * 4), 0);

    std::vector<png_bytep> row_ptrs(static_cast<std::size_t>(out_height));
    for (int y = 0; y < out_height; ++y) {
        row_ptrs[static_cast<std::size_t>(y)] = reinterpret_cast<png_bytep>(
            out_rgba.data() + static_cast<std::size_t>(y * out_width * 4));
    }
    png_read_image(png, row_ptrs.data());
    png_read_end(png, nullptr);

    png_destroy_read_struct(&png, &info, nullptr);
    std::fclose(file);
    return true;
}
#endif

void ensure_sticker_loaded(StickerTexture& texture, const std::filesystem::path& asset_path) {
    if (texture.load_attempted) {
        return;
    }
    texture.load_attempted = true;

#if defined(WHACKER_HAS_PNG)
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba {};
    if (!load_png_rgba(asset_path, width, height, rgba)) {
        texture.loaded = false;
        return;
    }
    texture.width = width;
    texture.height = height;
    texture.rgba = std::move(rgba);
    texture.loaded = true;
#else
    (void)asset_path;
    texture.loaded = false;
#endif
}

bool ensure_sticker_uploaded(StickerTexture& texture) {
    if (texture.uploaded && texture.texture_id != 0) {
        return true;
    }
    if (!texture.loaded || texture.width <= 0 || texture.height <= 0 || texture.rgba.empty()) {
        return false;
    }

    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);
    if (texture_id == 0) {
        return false;
    }

    GLint prev_unpack_alignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &prev_unpack_alignment);

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        texture.width,
        texture.height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        texture.rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, prev_unpack_alignment);

    texture.texture_id = texture_id;
    texture.uploaded = true;
    return true;
}

void draw_sticker_quad(
    const int fb_width,
    const int fb_height,
    const StickerTexture& texture,
    const MenuStickerRect& rect,
    const float rotation_deg) {
    if (texture.texture_id == 0 || rect.w <= 0.0f || rect.h <= 0.0f || fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const float center_x = rect.x + 0.5f * rect.w;
    const float center_y = rect.y + 0.5f * rect.h;
    const float half_w = 0.5f * rect.w;
    const float half_h = 0.5f * rect.h;
    const float radians = rotation_deg * (3.1415926535f / 180.0f);
    const float cos_a = std::cos(radians);
    const float sin_a = std::sin(radians);
    const auto rotate_pixel_point = [center_x, center_y, cos_a, sin_a](const float local_x, const float local_y) {
        return std::array<float, 2> {
            center_x + (local_x * cos_a) - (local_y * sin_a),
            center_y + (local_x * sin_a) + (local_y * cos_a),
        };
    };
    const std::array<std::array<float, 2>, 4> corners_px {{
        rotate_pixel_point(-half_w, -half_h),
        rotate_pixel_point(half_w, -half_h),
        rotate_pixel_point(half_w, half_h),
        rotate_pixel_point(-half_w, half_h),
    }};
    const auto to_ndc_x = [fb_width](const float x_px) {
        return (x_px / static_cast<float>(fb_width)) * 2.0f - 1.0f;
    };
    const auto to_ndc_y = [fb_height](const float y_px) {
        return 1.0f - (y_px / static_cast<float>(fb_height)) * 2.0f;
    };

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture.texture_id);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(to_ndc_x(corners_px[0][0]), to_ndc_y(corners_px[0][1]));
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(to_ndc_x(corners_px[1][0]), to_ndc_y(corners_px[1][1]));
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(to_ndc_x(corners_px[2][0]), to_ndc_y(corners_px[2][1]));
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(to_ndc_x(corners_px[3][0]), to_ndc_y(corners_px[3][1]));
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

MenuStickerRect anchored_sticker_rect(
    const MenuStickerRect& panel_rect,
    const MenuStickerSlotSpec& slot,
    const float sticker_w,
    const float sticker_h) {
    const float x_inset = kInsideInsetPx + slot.offset_x_px;
    const float y_inset = kInsideInsetPx + slot.offset_y_px;
    switch (slot.anchor) {
        case MenuStickerAnchor::TopLeft:
            return MenuStickerRect {
                .x = panel_rect.x - sticker_w + x_inset,
                .y = panel_rect.y - sticker_h + y_inset,
                .w = sticker_w,
                .h = sticker_h,
            };
        case MenuStickerAnchor::TopRight:
            return MenuStickerRect {
                .x = panel_rect.x + panel_rect.w - x_inset,
                .y = panel_rect.y - sticker_h + y_inset,
                .w = sticker_w,
                .h = sticker_h,
            };
        case MenuStickerAnchor::BottomLeft:
            return MenuStickerRect {
                .x = panel_rect.x - sticker_w + x_inset,
                .y = panel_rect.y + panel_rect.h - y_inset,
                .w = sticker_w,
                .h = sticker_h,
            };
        case MenuStickerAnchor::BottomRight:
            return MenuStickerRect {
                .x = panel_rect.x + panel_rect.w - x_inset,
                .y = panel_rect.y + panel_rect.h - y_inset,
                .w = sticker_w,
                .h = sticker_h,
            };
        case MenuStickerAnchor::LeftEdge:
            return MenuStickerRect {
                .x = panel_rect.x - sticker_w + x_inset,
                .y = panel_rect.y + 0.5f * (panel_rect.h - sticker_h) + slot.offset_y_px,
                .w = sticker_w,
                .h = sticker_h,
            };
        case MenuStickerAnchor::RightEdge:
            return MenuStickerRect {
                .x = panel_rect.x + panel_rect.w - x_inset,
                .y = panel_rect.y + 0.5f * (panel_rect.h - sticker_h) + slot.offset_y_px,
                .w = sticker_w,
                .h = sticker_h,
            };
        default:
            return MenuStickerRect {
                .x = panel_rect.x - sticker_w + x_inset,
                .y = panel_rect.y - sticker_h + y_inset,
                .w = sticker_w,
                .h = sticker_h,
            };
    }
}

MenuStickerRect inside_sticker_rect(
    const MenuStickerRect& panel_rect,
    const MenuStickerSlotSpec& slot,
    const float sticker_w,
    const float sticker_h) {
    return MenuStickerRect {
        .x = panel_rect.x + (panel_rect.w * slot.x_norm) - (0.5f * sticker_w) + slot.offset_x_px,
        .y = panel_rect.y + (panel_rect.h * slot.y_norm) - (0.5f * sticker_h) + slot.offset_y_px,
        .w = sticker_w,
        .h = sticker_h,
    };
}

void clamp_sticker_rect_to_viewport(const int fb_width, const int fb_height, MenuStickerRect& rect) {
    (void)kMinVisibleStickerPixels;
    rect.w = std::clamp(rect.w, 1.0f, static_cast<float>(fb_width));
    rect.h = std::clamp(rect.h, 1.0f, static_cast<float>(fb_height));
    const float min_x = 0.0f;
    const float max_x = std::max(0.0f, static_cast<float>(fb_width) - rect.w);
    const float min_y = 0.0f;
    const float max_y = std::max(0.0f, static_cast<float>(fb_height) - rect.h);
    rect.x = std::clamp(rect.x, min_x, max_x);
    rect.y = std::clamp(rect.y, min_y, max_y);
}

const MenuStickerSurfaceSpec* find_surface_spec(const MenuStickerSurface surface) {
    const std::span<const MenuStickerSurfaceSpec> surfaces = story_pack::menu_sticker_surfaces();
    for (const MenuStickerSurfaceSpec& entry : surfaces) {
        if (entry.surface == surface) {
            return &entry;
        }
    }
    return nullptr;
}

}  // namespace

void render_menu_stickers(
    const int fb_width,
    const int fb_height,
    const MenuStickerSurface surface,
    const MenuStickerRect& panel_rect,
    const std::span<const MenuStickerRect> protected_regions) {
    if (fb_width <= 0 || fb_height <= 0 || panel_rect.w <= 0.0f || panel_rect.h <= 0.0f) {
        return;
    }

    const MenuStickerSurfaceSpec* surface_spec = find_surface_spec(surface);
    if (surface_spec == nullptr || surface_spec->slots.empty()) {
        return;
    }

    const ScopedStickerScissor scoped_scissor {fb_width, fb_height};

    std::vector<const MenuStickerSlotSpec*> sorted_slots {};
    sorted_slots.reserve(surface_spec->slots.size());
    for (const MenuStickerSlotSpec& slot : surface_spec->slots) {
        sorted_slots.push_back(&slot);
    }
    std::stable_sort(
        sorted_slots.begin(),
        sorted_slots.end(),
        [](const MenuStickerSlotSpec* a, const MenuStickerSlotSpec* b) {
            return a->z_order < b->z_order;
        });

    std::vector<MenuStickerRect> placed_rects {};
    placed_rects.reserve(sorted_slots.size());

    auto& cache = sticker_texture_cache();
    for (const MenuStickerSlotSpec* slot_ptr : sorted_slots) {
        if (slot_ptr == nullptr) {
            continue;
        }
        const MenuStickerSlotSpec& slot = *slot_ptr;
        if (slot.asset_filename.empty()) {
            continue;
        }

        StickerTexture& texture = cache[std::string(slot.asset_filename)];
        ensure_sticker_loaded(texture, sticker_asset_path(slot.asset_filename));
        if (!ensure_sticker_uploaded(texture)) {
            continue;
        }

        const float texture_aspect = texture.height > 0
            ? (static_cast<float>(texture.width) / static_cast<float>(texture.height))
            : 1.0f;
        float sticker_w = 0.0f;
        float sticker_h = 0.0f;
        if (slot.size_mode == MenuStickerSizeMode::HeightPx) {
            sticker_h = std::clamp(slot.height_px, kMinStickerSizePx, kMaxStickerSizePx);
            sticker_w = sticker_h * texture_aspect;
        } else {
            const float desired_w = std::clamp(panel_rect.w * slot.scale, kMinStickerSizePx, kMaxStickerSizePx);
            sticker_w = desired_w;
            sticker_h = desired_w / std::max(1.0e-4f, texture_aspect);
        }
        const float max_w = panel_rect.w * 0.58f;
        const float max_h = panel_rect.h * 0.58f;
        if (sticker_w > max_w) {
            const float downscale = max_w / std::max(1.0e-4f, sticker_w);
            sticker_w = max_w;
            sticker_h *= downscale;
        }
        if (sticker_h > max_h) {
            const float downscale = max_h / std::max(1.0e-4f, sticker_h);
            sticker_h = max_h;
            sticker_w *= downscale;
        }
        sticker_w = std::min(sticker_w, static_cast<float>(fb_width));
        sticker_h = std::min(sticker_h, static_cast<float>(fb_height));
        if (sticker_w <= 0.0f || sticker_h <= 0.0f) {
            continue;
        }

        MenuStickerRect sticker_rect = slot.placement == MenuStickerPlacement::Inside
            ? inside_sticker_rect(panel_rect, slot, sticker_w, sticker_h)
            : anchored_sticker_rect(panel_rect, slot, sticker_w, sticker_h);
        clamp_sticker_rect_to_viewport(fb_width, fb_height, sticker_rect);

        if (!slot.allow_protected_overlap) {
            bool overlaps_protected = false;
            for (const MenuStickerRect& protected_rect : protected_regions) {
                if (overlap_area(sticker_rect, protected_rect) > kProtectedOverlapThresholdPx) {
                    overlaps_protected = true;
                    break;
                }
            }
            if (overlaps_protected) {
                continue;
            }
        }

        bool overlaps_existing = false;
        for (const MenuStickerRect& placed : placed_rects) {
            if (overlap_area(sticker_rect, placed) > kStickerOverlapThresholdPx) {
                overlaps_existing = true;
                break;
            }
        }
        if (overlaps_existing) {
            continue;
        }

        draw_sticker_quad(fb_width, fb_height, texture, sticker_rect, slot.rotation_deg);
        placed_rects.push_back(sticker_rect);
    }
}

}  // namespace whacker::app
