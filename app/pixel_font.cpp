#include "pixel_font.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include <GL/gl.h>

namespace {

struct PixelRenderViewport {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    bool active = false;
};

PixelRenderViewport& active_pixel_viewport() {
    static PixelRenderViewport viewport {};
    return viewport;
}

PixelRenderViewport pixel_viewport_for(const int fb_width, const int fb_height) {
    const PixelRenderViewport& viewport = active_pixel_viewport();
    if (viewport.active && viewport.width == fb_width && viewport.height == fb_height) {
        return viewport;
    }
    return PixelRenderViewport {
        .x = 0,
        .y = 0,
        .width = fb_width,
        .height = fb_height,
        .active = false,
    };
}

std::array<std::uint8_t, 5> glyph_3x5(char ch) {
    switch (ch) {
        case 'A': return {7, 5, 7, 5, 5};
        case 'B': return {6, 5, 6, 5, 6};
        case 'C': return {7, 4, 4, 4, 7};
        case 'D': return {6, 5, 5, 5, 6};
        case 'E': return {7, 4, 6, 4, 7};
        case 'F': return {7, 4, 6, 4, 4};
        case 'G': return {7, 4, 5, 5, 7};
        case 'H': return {5, 5, 7, 5, 5};
        case 'I': return {7, 2, 2, 2, 7};
        case 'J': return {1, 1, 1, 5, 7};
        case 'K': return {5, 5, 6, 5, 5};
        case 'L': return {4, 4, 4, 4, 7};
        case 'M': return {5, 7, 7, 5, 5};
        case 'N': return {5, 7, 7, 7, 5};
        case 'O': return {7, 5, 5, 5, 7};
        case 'P': return {7, 5, 7, 4, 4};
        case 'Q': return {7, 5, 5, 7, 1};
        case 'R': return {6, 5, 6, 5, 5};
        case 'S': return {7, 4, 7, 1, 7};
        case 'T': return {7, 2, 2, 2, 2};
        case 'U': return {5, 5, 5, 5, 7};
        case 'V': return {5, 5, 5, 5, 2};
        case 'W': return {5, 5, 7, 7, 5};
        case 'X': return {5, 5, 2, 5, 5};
        case 'Y': return {5, 5, 7, 2, 2};
        case 'Z': return {7, 1, 2, 4, 7};
        case 'a': return {0, 3, 5, 7, 5};
        case 'b': return {4, 4, 6, 5, 6};
        case 'c': return {0, 3, 4, 4, 3};
        case 'd': return {1, 1, 3, 5, 3};
        case 'e': return {0, 3, 7, 4, 3};
        case 'f': return {1, 2, 7, 2, 2};
        case 'g': return {0, 3, 5, 3, 1};
        case 'h': return {4, 4, 6, 5, 5};
        case 'i': return {2, 0, 6, 2, 7};
        case 'j': return {1, 0, 1, 5, 2};
        case 'k': return {4, 5, 6, 5, 5};
        case 'l': return {6, 2, 2, 2, 7};
        case 'm': return {0, 6, 7, 5, 5};
        case 'n': return {0, 6, 5, 5, 5};
        case 'o': return {0, 3, 5, 5, 3};
        case 'p': return {0, 6, 5, 6, 4};
        case 'q': return {0, 3, 5, 3, 1};
        case 'r': return {0, 6, 5, 4, 4};
        case 's': return {0, 3, 6, 3, 6};
        case 't': return {2, 7, 2, 2, 1};
        case 'u': return {0, 5, 5, 5, 3};
        case 'v': return {0, 5, 5, 5, 2};
        case 'w': return {0, 5, 7, 7, 5};
        case 'x': return {0, 5, 2, 2, 5};
        case 'y': return {0, 5, 3, 1, 6};
        case 'z': return {0, 7, 1, 2, 7};
        case '0': return {7, 5, 5, 5, 7};
        case '1': return {2, 6, 2, 2, 7};
        case '2': return {7, 1, 7, 4, 7};
        case '3': return {7, 1, 7, 1, 7};
        case '4': return {5, 5, 7, 1, 1};
        case '5': return {7, 4, 7, 1, 7};
        case '6': return {7, 4, 7, 5, 7};
        case '7': return {7, 1, 1, 1, 1};
        case '8': return {7, 5, 7, 5, 7};
        case '9': return {7, 5, 7, 1, 7};
        case ':': return {0, 2, 0, 2, 0};
        case ';': return {0, 2, 0, 2, 4};
        case '!': return {2, 2, 2, 0, 2};
        case '?': return {7, 1, 3, 0, 2};
        case '-': return {0, 0, 7, 0, 0};
        case '_': return {0, 0, 0, 0, 7};
        case '=': return {0, 7, 0, 7, 0};
        case '+': return {0, 2, 7, 2, 0};
        case '/': return {1, 1, 2, 4, 4};
        case '\\': return {4, 4, 2, 1, 1};
        case '.': return {0, 0, 0, 0, 2};
        case ',': return {0, 0, 0, 2, 4};
        case '\'': return {2, 2, 0, 0, 0};
        case '"': return {5, 5, 0, 0, 0};
        case '(': return {1, 2, 2, 2, 1};
        case ')': return {4, 2, 2, 2, 4};
        case '[': return {3, 2, 2, 2, 3};
        case ']': return {6, 2, 2, 2, 6};
        case '|': return {2, 2, 2, 2, 2};
        case '%': return {5, 1, 2, 4, 5};
        case '*': return {0, 5, 2, 5, 0};
        case '>': return {4, 2, 1, 2, 4};
        case '<': return {1, 2, 4, 2, 1};
        default: return {0, 0, 0, 0, 0};
    }
}

float text_pixel_width(const std::string& text, const float scale) {
    if (text.empty()) {
        return 0.0f;
    }
    return static_cast<float>(text.size()) * (3.0f * scale) + static_cast<float>(text.size() - 1) * scale;
}

void draw_digit_pixels(
    const int fb_width,
    const int fb_height,
    const float x,
    const float y,
    const float scale,
    const int digit,
    const whacker::app::Color color) {
    if (digit < 0 || digit > 9) {
        return;
    }

    static constexpr std::array<std::array<int, 7>, 10> kSegments {
        std::array<int, 7> {1, 1, 1, 0, 1, 1, 1},
        std::array<int, 7> {0, 0, 1, 0, 0, 1, 0},
        std::array<int, 7> {1, 0, 1, 1, 1, 0, 1},
        std::array<int, 7> {1, 0, 1, 1, 0, 1, 1},
        std::array<int, 7> {0, 1, 1, 1, 0, 1, 0},
        std::array<int, 7> {1, 1, 0, 1, 0, 1, 1},
        std::array<int, 7> {1, 1, 0, 1, 1, 1, 1},
        std::array<int, 7> {1, 0, 1, 0, 0, 1, 0},
        std::array<int, 7> {1, 1, 1, 1, 1, 1, 1},
        std::array<int, 7> {1, 1, 1, 1, 0, 1, 1},
    };

    const float thickness = 2.0f * scale;
    const float length = 6.0f * scale;
    const auto& segments = kSegments[static_cast<std::size_t>(digit)];

    if (segments[0] != 0) {
        whacker::app::draw_rect_pixels(fb_width, fb_height, x + thickness, y, length, thickness, color.r, color.g, color.b);
    }
    if (segments[1] != 0) {
        whacker::app::draw_rect_pixels(
            fb_width,
            fb_height,
            x,
            y + thickness,
            thickness,
            3.0f * scale,
            color.r,
            color.g,
            color.b);
    }
    if (segments[2] != 0) {
        whacker::app::draw_rect_pixels(
            fb_width,
            fb_height,
            x + thickness + length,
            y + thickness,
            thickness,
            3.0f * scale,
            color.r,
            color.g,
            color.b);
    }
    if (segments[3] != 0) {
        whacker::app::draw_rect_pixels(
            fb_width,
            fb_height,
            x + thickness,
            y + 4.0f * scale,
            length,
            thickness,
            color.r,
            color.g,
            color.b);
    }
    if (segments[4] != 0) {
        whacker::app::draw_rect_pixels(
            fb_width,
            fb_height,
            x,
            y + 5.0f * scale,
            thickness,
            3.0f * scale,
            color.r,
            color.g,
            color.b);
    }
    if (segments[5] != 0) {
        whacker::app::draw_rect_pixels(
            fb_width,
            fb_height,
            x + thickness + length,
            y + 5.0f * scale,
            thickness,
            3.0f * scale,
            color.r,
            color.g,
            color.b);
    }
    if (segments[6] != 0) {
        whacker::app::draw_rect_pixels(
            fb_width,
            fb_height,
            x + thickness,
            y + 8.0f * scale,
            length,
            thickness,
            color.r,
            color.g,
            color.b);
    }
}

}  // namespace

namespace whacker::app {

ScopedPixelRenderContext::ScopedPixelRenderContext(const RenderContext& context) {
    PixelRenderViewport& viewport = active_pixel_viewport();
    if (!render_context_valid(context)) {
        viewport = PixelRenderViewport {};
        return;
    }
    viewport = PixelRenderViewport {
        .x = context.viewport_x,
        .y = context.viewport_y,
        .width = context.framebuffer_width,
        .height = context.framebuffer_height,
        .active = true,
    };
}

ScopedPixelRenderContext::~ScopedPixelRenderContext() {
    active_pixel_viewport() = PixelRenderViewport {};
}

void apply_render_context_viewport(const RenderContext& context) {
    if (!render_context_valid(context)) {
        return;
    }
    glViewport(
        context.viewport_x,
        context.viewport_y,
        context.framebuffer_width,
        context.framebuffer_height);
}

void apply_full_pixel_scissor(const int fb_width, const int fb_height) {
    const PixelRenderViewport viewport = pixel_viewport_for(fb_width, fb_height);
    glScissor(viewport.x, viewport.y, fb_width, fb_height);
}

void draw_rect_pixels(
    const int fb_width,
    const int fb_height,
    const float x,
    const float y,
    const float w,
    const float h,
    const float r,
    const float g,
    const float b) {
    const int sx = std::max(0, static_cast<int>(std::lround(x)));
    const int sy_top = std::max(0, static_cast<int>(std::lround(y)));
    const int sw = std::min(std::max(0, static_cast<int>(std::lround(w))), std::max(0, fb_width - sx));
    const int sh = std::min(std::max(0, static_cast<int>(std::lround(h))), std::max(0, fb_height - sy_top));
    if (sw <= 0 || sh <= 0 || fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const int sy = fb_height - (sy_top + sh);
    if (sx >= fb_width || sy >= fb_height || (sx + sw) <= 0 || (sy + sh) <= 0) {
        return;
    }

    const PixelRenderViewport viewport = pixel_viewport_for(fb_width, fb_height);
    glScissor(viewport.x + sx, viewport.y + sy, sw, sh);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void draw_text_pixels(
    const int fb_width,
    const int fb_height,
    const float x,
    const float y,
    const float scale,
    const std::string& text,
    const Color color) {
    float cursor_x = x;
    for (const char ch : text) {
        const std::array<std::uint8_t, 5> rows = glyph_3x5(ch);
        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 3; ++col) {
                const std::uint8_t bit = static_cast<std::uint8_t>(1u << (2 - col));
                if ((rows[static_cast<std::size_t>(row)] & bit) == 0) {
                    continue;
                }
                draw_rect_pixels(
                    fb_width,
                    fb_height,
                    cursor_x + static_cast<float>(col) * scale,
                    y + static_cast<float>(row) * scale,
                    scale,
                    scale,
                    color.r,
                    color.g,
                    color.b);
            }
        }
        cursor_x += 4.0f * scale;
    }
}

void draw_text_centered(
    const int fb_width,
    const int fb_height,
    const float x,
    const float y,
    const float w,
    const float h,
    const float scale,
    const std::string& text,
    const Color color) {
    const float text_w = text_pixel_width(text, scale);
    const float text_h = 5.0f * scale;
    const float tx = x + std::max(0.0f, 0.5f * (w - text_w));
    const float ty = y + std::max(0.0f, 0.5f * (h - text_h));
    draw_text_pixels(fb_width, fb_height, tx, ty, scale, text, color);
}

float text_char_advance_pixels(const float scale) {
    return 4.0f * scale;
}

float text_line_height_pixels(const float scale) {
    return 5.0f * scale;
}

void draw_two_digits(
    const int fb_width,
    const int fb_height,
    const float x,
    const float y,
    const float scale,
    const int value,
    const Color color) {
    const int clamped = std::clamp(value, 0, 99);
    const int tens = clamped / 10;
    const int ones = clamped % 10;
    draw_digit_pixels(fb_width, fb_height, x, y, scale, tens, color);
    draw_digit_pixels(fb_width, fb_height, x + (12.0f * scale), y, scale, ones, color);
}

}  // namespace whacker::app
