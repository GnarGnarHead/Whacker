#include "runtime_visual_transition_render.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <utility>
#include <vector>

#include <GL/gl.h>

#include "pixel_font.hpp"

#if defined(WHACKER_HAS_PNG)
#include <png.h>
#endif

#ifndef WHACKER_SOURCE_DIR
#define WHACKER_SOURCE_DIR "."
#endif

namespace whacker::app {

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kStarInnerRadiusRatio = 0.45f;
constexpr float kStarFrameScale = 2.45f;
constexpr const char* kStarFrameSourceFilename = "magical_star_wipe_frame_source.png";

struct WipeFrameTexture {
    bool load_attempted = false;
    bool loaded = false;
    bool uploaded = false;
    int width = 0;
    int height = 0;
    GLuint texture_id = 0;
    std::vector<std::uint8_t> rgba {};
};

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

WipeFrameTexture& wipe_frame_texture_cache() {
    static WipeFrameTexture cache {};
    return cache;
}

float smoothstep01(const float t) {
    const float clamped = std::clamp(t, 0.0f, 1.0f);
    return clamped * clamped * (3.0f - (2.0f * clamped));
}

float aperture_open_ratio_for_progress(const float progress) {
    const float eased = smoothstep01(progress);
    if (eased < 0.5f) {
        return 1.0f - (eased * 2.0f);
    }
    return (eased - 0.5f) * 2.0f;
}

std::filesystem::path star_frame_source_path() {
    return std::filesystem::path(WHACKER_SOURCE_DIR) / "story" / "art" / kStarFrameSourceFilename;
}

void begin_pixel_projection(const int fb_width, const int fb_height) {
    apply_full_pixel_scissor(fb_width, fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(fb_width), static_cast<double>(fb_height), 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void end_pixel_projection() {
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void draw_fullscreen_black_quad(const int fb_width, const int fb_height, const float alpha) {
    const float safe_alpha = std::clamp(alpha, 0.0f, 1.0f);
    if (safe_alpha <= 0.0f) {
        return;
    }
    glColor4f(0.0f, 0.0f, 0.0f, safe_alpha);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(fb_width), 0.0f);
    glVertex2f(static_cast<float>(fb_width), static_cast<float>(fb_height));
    glVertex2f(0.0f, static_cast<float>(fb_height));
    glEnd();
}

void compute_star_points(
    const float cx,
    const float cy,
    const float outer_radius,
    const float inner_radius,
    std::array<Vec2, 5>& outer_points,
    std::array<Vec2, 5>& inner_points) {
    for (int i = 0; i < 5; ++i) {
        const float outer_angle = (-0.5f * kPi) + (static_cast<float>(i) * (2.0f * kPi / 5.0f));
        const float inner_angle = outer_angle + (kPi / 5.0f);
        outer_points[static_cast<std::size_t>(i)] = Vec2 {
            cx + std::cos(outer_angle) * outer_radius,
            cy + std::sin(outer_angle) * outer_radius};
        inner_points[static_cast<std::size_t>(i)] = Vec2 {
            cx + std::cos(inner_angle) * inner_radius,
            cy + std::sin(inner_angle) * inner_radius};
    }
}

void draw_filled_star_shape(const float cx, const float cy, const float outer_radius, const float inner_radius) {
    std::array<Vec2, 5> outer_points {};
    std::array<Vec2, 5> inner_points {};
    compute_star_points(cx, cy, outer_radius, inner_radius, outer_points, inner_points);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i < 5; ++i) {
        const Vec2& v = inner_points[static_cast<std::size_t>(i)];
        glVertex2f(v.x, v.y);
    }
    const Vec2& first = inner_points[0];
    glVertex2f(first.x, first.y);
    glEnd();

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; ++i) {
        const int prev_inner_idx = (i + 4) % 5;
        const Vec2& outer = outer_points[static_cast<std::size_t>(i)];
        const Vec2& prev_inner = inner_points[static_cast<std::size_t>(prev_inner_idx)];
        const Vec2& next_inner = inner_points[static_cast<std::size_t>(i)];
        glVertex2f(outer.x, outer.y);
        glVertex2f(next_inner.x, next_inner.y);
        glVertex2f(prev_inner.x, prev_inner.y);
    }
    glEnd();
}

bool draw_black_outside_star_aperture(
    const int fb_width,
    const int fb_height,
    const float cx,
    const float cy,
    const float outer_radius,
    const float inner_radius) {
    GLint stencil_bits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);
    if (stencil_bits <= 0) {
        return false;
    }

    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    draw_filled_star_shape(cx, cy, outer_radius, inner_radius);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_fullscreen_black_quad(fb_width, fb_height, 1.0f);
    glDisable(GL_BLEND);

    glStencilMask(0xFF);
    glDisable(GL_STENCIL_TEST);
    return true;
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

void ensure_wipe_frame_loaded(WipeFrameTexture& frame_texture) {
    if (frame_texture.load_attempted) {
        return;
    }
    frame_texture.load_attempted = true;

#if defined(WHACKER_HAS_PNG)
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba {};
    if (!load_png_rgba(star_frame_source_path(), width, height, rgba)) {
        frame_texture.loaded = false;
        return;
    }
    frame_texture.width = width;
    frame_texture.height = height;
    frame_texture.rgba = std::move(rgba);
    frame_texture.loaded = true;
#else
    frame_texture.loaded = false;
#endif
}

bool ensure_wipe_frame_uploaded(WipeFrameTexture& frame_texture) {
    if (frame_texture.uploaded && frame_texture.texture_id != 0) {
        return true;
    }
    if (!frame_texture.loaded || frame_texture.width <= 0 || frame_texture.height <= 0 || frame_texture.rgba.empty()) {
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
        frame_texture.width,
        frame_texture.height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        frame_texture.rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, prev_unpack_alignment);

    frame_texture.texture_id = texture_id;
    frame_texture.uploaded = true;
    return true;
}

void draw_wipe_frame_overlay(
    const float cx,
    const float cy,
    const float outer_radius,
    const float alpha,
    WipeFrameTexture& frame_texture) {
    ensure_wipe_frame_loaded(frame_texture);
    if (!ensure_wipe_frame_uploaded(frame_texture)) {
        return;
    }

    const float safe_alpha = std::clamp(alpha, 0.0f, 1.0f);
    if (safe_alpha <= 0.0f) {
        return;
    }

    const float size = std::max(1.0f, outer_radius * kStarFrameScale);
    const float aspect = frame_texture.height > 0 ? static_cast<float>(frame_texture.width) / static_cast<float>(frame_texture.height)
                                                  : 1.0f;
    const float width = size;
    const float height = width / std::max(1.0e-4f, aspect);
    const float x = cx - (0.5f * width);
    const float y = cy - (0.5f * height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, frame_texture.texture_id);
    glColor4f(1.0f, 1.0f, 1.0f, safe_alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x + width, y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x + width, y + height);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x, y + height);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

}  // namespace

void render_visual_transition_overlay(const RenderContext& context, const RuntimeVisualTransitionState& transition) {
    if (!transition.active) {
        return;
    }

    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const float progress = visual_transition_progress(transition);
    const float open_ratio = std::clamp(aperture_open_ratio_for_progress(progress), 0.0f, 1.0f);

    const float cx = 0.5f * static_cast<float>(fb_width);
    const float cy = 0.5f * static_cast<float>(fb_height);
    const float max_radius = std::sqrt(
        static_cast<float>(fb_width * fb_width + fb_height * fb_height)) *
        1.35f;
    const float min_radius = 10.0f;
    const float outer_radius = min_radius + (open_ratio * (max_radius - min_radius));
    const float inner_radius = std::max(1.0f, outer_radius * kStarInnerRadiusRatio);

    begin_pixel_projection(fb_width, fb_height);
    const bool star_aperture_masked =
        draw_black_outside_star_aperture(fb_width, fb_height, cx, cy, outer_radius, inner_radius);
    if (!star_aperture_masked) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        draw_fullscreen_black_quad(fb_width, fb_height, 1.0f - open_ratio);
        glDisable(GL_BLEND);
    }

    WipeFrameTexture& frame_texture = wipe_frame_texture_cache();
    draw_wipe_frame_overlay(cx, cy, outer_radius, 1.0f, frame_texture);
    end_pixel_projection();
}

void release_visual_transition_render_resources() {
    WipeFrameTexture& frame_texture = wipe_frame_texture_cache();
    if (frame_texture.uploaded && frame_texture.texture_id != 0) {
        glDeleteTextures(1, &frame_texture.texture_id);
    }
    frame_texture = WipeFrameTexture {};
}

}  // namespace whacker::app
