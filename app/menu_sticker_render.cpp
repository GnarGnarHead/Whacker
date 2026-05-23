#include "menu_sticker_render.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <GL/gl.h>

#include "pixel_font.hpp"
#include "png_rgba_loader.hpp"
#include "rgba_texture.hpp"
#include "runtime_asset_path.hpp"
#include "story_pack.hpp"

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
    bool upload_attempted = false;
    int width = 0;
    int height = 0;
    RgbaTexture texture {};
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
    return runtime_asset_path(std::filesystem::path("story") / "art" / "Stickers" / std::string(filename));
}

void ensure_sticker_loaded(StickerTexture& texture, const std::filesystem::path& asset_path) {
    if (texture.load_attempted) {
        return;
    }
    texture.load_attempted = true;

    PngRgbaImage image {};
    if (!load_png_rgba_image(asset_path, image, "menu sticker")) {
        texture.loaded = false;
        return;
    }
    texture.width = image.width;
    texture.height = image.height;
    texture.rgba = std::move(image.rgba);
    texture.loaded = true;
}

bool ensure_sticker_uploaded(StickerTexture& texture) {
    if (texture.texture.texture_id != 0) {
        return true;
    }
    if (texture.upload_attempted) {
        return false;
    }
    if (!texture.loaded || texture.width <= 0 || texture.height <= 0 || texture.rgba.empty()) {
        return false;
    }

    texture.upload_attempted = true;
    const RgbaTextureUploadResult result =
        upload_rgba_texture(texture.rgba.data(), texture.width, texture.height, "menu sticker");
    if (!result.uploaded) {
        return false;
    }
    texture.texture = result.texture;
    std::vector<std::uint8_t> {}.swap(texture.rgba);
    return texture.texture.texture_id != 0;
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

        draw_rgba_texture_quad_pixels(
            fb_width,
            fb_height,
            texture.texture,
            sticker_rect.x,
            sticker_rect.y,
            sticker_rect.w,
            sticker_rect.h,
            1.0f,
            1.0f,
            false,
            slot.rotation_deg);
        placed_rects.push_back(sticker_rect);
    }
}

}  // namespace whacker::app
