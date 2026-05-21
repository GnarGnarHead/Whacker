#include "main_menu_overlay.hpp"

#include <algorithm>
#include <array>
#include <initializer_list>
#include <string>
#include <string_view>

#include "menu_sticker_render.hpp"
#include "overlay_layout_math.hpp"
#include "pixel_font.hpp"
#include "platform_sdl.hpp"
#include "text_wrap.hpp"
#include "ui_text.hpp"

namespace whacker::app {

namespace {

float centered_text_y(const float box_y, const float box_h, const float scale) {
    return box_y + std::max(0.0f, 0.5f * (box_h - text_line_height_pixels(scale)));
}

std::string fit_for_width(
    const std::string_view text,
    const float width_px,
    const float scale,
    const int reserved_chars = 0) {
    return fit_text_to_single_line_ellipsis(
        std::string(text),
        max_chars_for_text_width(width_px, scale, reserved_chars));
}

std::string choose_variant_for_width(
    const std::initializer_list<std::string_view> variants,
    const float width_px,
    const float scale,
    const int reserved_chars = 0) {
    return choose_best_fitting_variant(
        variants,
        max_chars_for_text_width(width_px, scale, reserved_chars));
}

}  // namespace

void render_main_menu_overlay(
    SdlPlatform* platform,
    const MainMenuState& menu_state,
    const MainMenuRowNameFn row_name_fn) {
    int fb_width = 0;
    int fb_height = 0;
    if (platform != nullptr) {
        platform->framebuffer_size(fb_width, fb_height);
    }
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const auto safe_row_name = [row_name_fn](const int row) -> const char* {
        return row_name_fn != nullptr ? row_name_fn(row) : ui_text::unknown_label();
    };

    const float panel_x = static_cast<float>(fb_width) * 0.24f;
    const float panel_y = static_cast<float>(fb_height) * 0.16f;
    const float panel_w = static_cast<float>(fb_width) * 0.52f;
    const float panel_h = static_cast<float>(fb_height) * 0.66f;
    const MenuStickerRect panel_rect {panel_x, panel_y, panel_w, panel_h};
    render_menu_stickers(
        fb_width,
        fb_height,
        MenuStickerSurface::MainMenuBack,
        panel_rect,
        {});
    draw_rect_pixels(fb_width, fb_height, panel_x, panel_y, panel_w, panel_h, 0.05f, 0.09f, 0.14f);
    draw_rect_pixels(fb_width, fb_height, panel_x + 4.0f, panel_y + 4.0f, panel_w - 8.0f, 44.0f, 0.09f, 0.16f, 0.24f);

    constexpr float kTitleScale = 3.0f;
    constexpr float kSubtitleScale = 1.8f;
    constexpr float kFooterScale = 2.0f;
    const OverlayVerticalLayout vertical = make_overlay_vertical_layout(
        panel_y,
        panel_h,
        72.0f,
        text_line_height_pixels(kFooterScale),
        8.0f,
        10.0f,
        8.0f);

    const std::string title = fit_for_width(ui_text::main_menu_title(), panel_w - 32.0f, kTitleScale);
    const std::string subtitle = fit_for_width(ui_text::main_menu_subtitle(), panel_w - 32.0f, kSubtitleScale);
    draw_text_pixels(
        fb_width,
        fb_height,
        panel_x + 16.0f,
        vertical.header_y + 8.0f,
        kTitleScale,
        title,
        Color {0.90f, 0.94f, 1.00f});
    draw_text_pixels(
        fb_width,
        fb_height,
        panel_x + 16.0f,
        vertical.header_y + 42.0f,
        kSubtitleScale,
        subtitle,
        Color {0.72f, 0.80f, 0.90f});

    const float row_x = panel_x + 18.0f;
    const float row_w = panel_w - 36.0f;
    const OverlayRowLayout rows = make_overlay_row_layout(
        vertical.body_y,
        vertical.body_h,
        MainMenuRowCount,
        52.0f,
        34.0f,
        10.0f,
        4.0f);
    const float rows_total_h =
        rows.row_h * static_cast<float>(MainMenuRowCount) +
        rows.row_gap * static_cast<float>(std::max(0, MainMenuRowCount - 1));
    constexpr float kRowLabelScale = 2.4f;

    for (int row = 0; row < MainMenuRowCount; ++row) {
        const bool selected = row == menu_state.selected_row;
        const float y = rows.row_start_y + static_cast<float>(row) * (rows.row_h + rows.row_gap);
        const std::string row_label = fit_for_width(safe_row_name(row), row_w - 44.0f, kRowLabelScale);
        draw_rect_pixels(
            fb_width,
            fb_height,
            row_x,
            y,
            row_w,
            rows.row_h,
            selected ? 0.20f : 0.12f,
            selected ? 0.30f : 0.18f,
            selected ? 0.40f : 0.24f);
        draw_text_pixels(
            fb_width,
            fb_height,
            row_x + 10.0f,
            centered_text_y(y, rows.row_h, kRowLabelScale),
            kRowLabelScale,
            selected ? ui_text::selected_marker() : ui_text::unselected_marker(),
            Color {0.96f, 0.86f, 0.34f});
        draw_text_pixels(
            fb_width,
            fb_height,
            row_x + 30.0f,
            centered_text_y(y, rows.row_h, kRowLabelScale),
            kRowLabelScale,
            row_label,
            Color {0.90f, 0.94f, 1.00f});
    }

    const std::string footer = choose_variant_for_width(
        {ui_text::main_menu_footer(), ui_text::main_menu_footer_short()},
        panel_w - 32.0f,
        kFooterScale);
    draw_text_pixels(
        fb_width,
        fb_height,
        panel_x + 16.0f,
        vertical.footer_y,
        kFooterScale,
        footer,
        Color {0.70f, 0.78f, 0.88f});

    const float header_protected_w = std::clamp(panel_w * 0.44f, 210.0f, panel_w - 28.0f);
    const std::array<MenuStickerRect, 3> protected_regions {{
        MenuStickerRect {panel_x + 14.0f, vertical.header_y + 6.0f, header_protected_w, 58.0f},
        MenuStickerRect {row_x + 20.0f, rows.row_start_y, row_w - 40.0f, rows_total_h},
        MenuStickerRect {
            panel_x + 14.0f,
            vertical.footer_y - 4.0f,
            panel_w - 28.0f,
            text_line_height_pixels(kFooterScale) + 10.0f,
        },
    }};
    render_menu_stickers(
        fb_width,
        fb_height,
        MenuStickerSurface::MainMenu,
        panel_rect,
        protected_regions);
}

}  // namespace whacker::app
