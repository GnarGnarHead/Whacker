#include "quick_menu_render.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <initializer_list>
#include <string>
#include <string_view>

#include "ai_style_catalog.hpp"
#include "menu_sticker_render.hpp"
#include "overlay_layout_math.hpp"
#include "paddle_tuning.hpp"
#include "pixel_font.hpp"
#include "text_wrap.hpp"
#include "ui_text.hpp"

namespace {

std::string uppercase_copy(const char* text) {
    std::string output = text != nullptr ? std::string(text) : std::string {};
    for (char& ch : output) {
        const unsigned char uch = static_cast<unsigned char>(ch);
        ch = static_cast<char>(std::toupper(uch));
    }
    return output;
}

std::string paddle_tuning_style_name(const whacker::progression::SkillState& skills) {
    return uppercase_copy(whacker::app::ai_style_display_name(whacker::app::style_for_skills(skills)));
}

float centered_text_y(const float box_y, const float box_h, const float scale) {
    return box_y + std::max(0.0f, 0.5f * (box_h - whacker::app::text_line_height_pixels(scale)));
}

std::string fit_for_width(
    const std::string_view text,
    const float width_px,
    const float scale,
    const int reserved_chars = 0) {
    return whacker::app::fit_text_to_single_line_ellipsis(
        std::string(text),
        whacker::app::max_chars_for_text_width(width_px, scale, reserved_chars));
}

std::string choose_variant_for_width(
    const std::initializer_list<std::string_view> variants,
    const float width_px,
    const float scale,
    const int reserved_chars = 0) {
    return whacker::app::choose_best_fitting_variant(
        variants,
        whacker::app::max_chars_for_text_width(width_px, scale, reserved_chars));
}

void draw_binary_option_row(
    const int fb_width,
    const int fb_height,
    const float x,
    const float y,
    const float w,
    const float h,
    const bool left_selected,
    const bool row_selected,
    const whacker::app::Color tint) {
    const float split = w * 0.5f;
    const float border = row_selected ? 2.0f : 1.0f;
    whacker::app::draw_rect_pixels(fb_width, fb_height, x, y, w, h, 0.16f, 0.20f, 0.25f);
    whacker::app::draw_rect_pixels(
        fb_width,
        fb_height,
        x + border,
        y + border,
        split - border * 1.5f,
        h - border * 2.0f,
        left_selected ? tint.r : 0.12f,
        left_selected ? tint.g : 0.15f,
        left_selected ? tint.b : 0.19f);
    whacker::app::draw_rect_pixels(
        fb_width,
        fb_height,
        x + split + border * 0.5f,
        y + border,
        split - border * 1.5f,
        h - border * 2.0f,
        left_selected ? 0.12f : tint.r,
        left_selected ? 0.15f : tint.g,
        left_selected ? 0.19f : tint.b);
}

}  // namespace

namespace whacker::app {

void render_menu_overlay(const RenderContext& context, const MatchOptions& options, const MenuState& menu_state) {
    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const float panel_x = static_cast<float>(fb_width) * 0.16f;
    const float panel_y = static_cast<float>(fb_height) * 0.16f;
    const float panel_w = static_cast<float>(fb_width) * 0.68f;
    const float panel_h = static_cast<float>(fb_height) * 0.70f;
    draw_rect_pixels(fb_width, fb_height, panel_x, panel_y, panel_w, panel_h, 0.05f, 0.09f, 0.14f);
    draw_rect_pixels(fb_width, fb_height, panel_x + 4.0f, panel_y + 4.0f, panel_w - 8.0f, 44.0f, 0.09f, 0.16f, 0.24f);

    constexpr float kTitleScale = 3.0f;
    constexpr float kFooterScale = 2.0f;
    const whacker::app::OverlayVerticalLayout vertical = whacker::app::make_overlay_vertical_layout(
        panel_y,
        panel_h,
        58.0f,
        whacker::app::text_line_height_pixels(kFooterScale),
        8.0f,
        10.0f,
        8.0f);
    const std::string title = fit_for_width(ui_text::quick_menu_title(), panel_w - 32.0f, kTitleScale);
    draw_text_pixels(
        fb_width,
        fb_height,
        panel_x + 16.0f,
        vertical.header_y + 8.0f,
        kTitleScale,
        title,
        Color {0.90f, 0.94f, 1.00f});

    const std::string footer = choose_variant_for_width(
        {ui_text::quick_menu_footer(), ui_text::quick_menu_footer_short()},
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

    const float row_x = panel_x + 20.0f;
    const float row_w = panel_w - 40.0f;
    const whacker::app::OverlayRowLayout rows = whacker::app::make_overlay_row_layout(
        vertical.body_y,
        vertical.body_h,
        MenuRowCount,
        40.0f,
        30.0f,
        8.0f,
        4.0f);
    const float rows_total_h =
        rows.row_h * static_cast<float>(MenuRowCount) +
        rows.row_gap * static_cast<float>(std::max(0, MenuRowCount - 1));
    const float option_w = std::clamp(row_w * 0.34f, 170.0f, 240.0f);

    for (int row = 0; row < MenuRowCount; ++row) {
        const float y = rows.row_start_y + (rows.row_h + rows.row_gap) * static_cast<float>(row);
        const bool selected = row == menu_state.selected_row;
        const float label_scale = rows.row_h < 34.0f ? 2.0f : 2.4f;
        const float option_h = std::clamp(rows.row_h - 10.0f, 22.0f, 30.0f);
        const float option_x = row_x + row_w - option_w - 12.0f;
        const float option_y = y + std::max(0.0f, 0.5f * (rows.row_h - option_h));
        draw_rect_pixels(
            fb_width,
            fb_height,
            row_x,
            y,
            row_w,
            rows.row_h,
            selected ? 0.18f : 0.11f,
            selected ? 0.26f : 0.16f,
            selected ? 0.34f : 0.20f);

        const float marker_x = row_x + 8.0f;
        const float marker_y = centered_text_y(y, rows.row_h, label_scale);
        draw_text_pixels(
            fb_width,
            fb_height,
            marker_x,
            marker_y,
            label_scale,
            selected ? ui_text::selected_marker() : ui_text::unselected_marker(),
            Color {0.96f, 0.86f, 0.34f});

        const char* label = "";
        if (row == MenuRowP1) {
            label = ui_text::quick_menu_row_p1_mode();
        } else if (row == MenuRowP2) {
            label = ui_text::quick_menu_row_p2_mode();
        } else if (row == MenuRowP1Tuning) {
            label = ui_text::quick_menu_row_p1_style();
        } else if (row == MenuRowP2Tuning) {
            label = ui_text::quick_menu_row_p2_style();
        } else if (row == MenuRowStart) {
            label = ui_text::quick_menu_row_start_match();
        }
        const std::string fitted_label = fit_for_width(label, row_w - option_w - 54.0f, label_scale);
        draw_text_pixels(
            fb_width,
            fb_height,
            row_x + 26.0f,
            centered_text_y(y, rows.row_h, label_scale),
            label_scale,
            fitted_label,
            Color {0.88f, 0.92f, 0.98f});

        if (row == MenuRowP1) {
            draw_binary_option_row(
                fb_width,
                fb_height,
                option_x,
                option_y,
                option_w,
                option_h,
                options.left_mode == PaddleMode::Human,
                selected,
                Color {0.28f, 0.75f, 0.39f});
            const float option_scale = option_h < 26.0f ? 1.6f : 1.9f;
            draw_text_centered(
                fb_width,
                fb_height,
                option_x + 3.0f,
                option_y + 2.0f,
                (option_w * 0.5f) - 6.0f,
                option_h - 4.0f,
                option_scale,
                ui_text::quick_menu_option_human(),
                Color {0.94f, 0.97f, 1.00f});
            draw_text_centered(
                fb_width,
                fb_height,
                option_x + (option_w * 0.5f) + 3.0f,
                option_y + 2.0f,
                (option_w * 0.5f) - 6.0f,
                option_h - 4.0f,
                option_scale,
                ui_text::quick_menu_option_ai(),
                Color {0.94f, 0.97f, 1.00f});
        } else if (row == MenuRowP2) {
            draw_binary_option_row(
                fb_width,
                fb_height,
                option_x,
                option_y,
                option_w,
                option_h,
                options.right_mode == PaddleMode::Human,
                selected,
                Color {0.28f, 0.70f, 0.86f});
            const float option_scale = option_h < 26.0f ? 1.6f : 1.9f;
            draw_text_centered(
                fb_width,
                fb_height,
                option_x + 3.0f,
                option_y + 2.0f,
                (option_w * 0.5f) - 6.0f,
                option_h - 4.0f,
                option_scale,
                ui_text::quick_menu_option_human(),
                Color {0.94f, 0.97f, 1.00f});
            draw_text_centered(
                fb_width,
                fb_height,
                option_x + (option_w * 0.5f) + 3.0f,
                option_y + 2.0f,
                (option_w * 0.5f) - 6.0f,
                option_h - 4.0f,
                option_scale,
                ui_text::quick_menu_option_ai(),
                Color {0.94f, 0.97f, 1.00f});
        } else if (row == MenuRowP1Tuning || row == MenuRowP2Tuning) {
            const std::string raw_style_label = row == MenuRowP1Tuning
                ? paddle_tuning_style_name(options.left_paddle_skills)
                : paddle_tuning_style_name(options.right_paddle_skills);
            const float option_scale = option_h < 26.0f ? 1.6f : 1.8f;
            const std::string style_label = fit_for_width(raw_style_label, option_w - 8.0f, option_scale);
            draw_rect_pixels(
                fb_width,
                fb_height,
                option_x,
                option_y,
                option_w,
                option_h,
                selected ? 0.28f : 0.16f,
                selected ? 0.40f : 0.24f,
                selected ? 0.60f : 0.34f);
            draw_text_centered(
                fb_width,
                fb_height,
                option_x,
                option_y,
                option_w,
                option_h,
                option_scale,
                style_label,
                Color {0.94f, 0.97f, 1.00f});
        } else if (row == MenuRowStart) {
            const float option_scale = option_h < 26.0f ? 1.7f : 2.0f;
            draw_rect_pixels(
                fb_width,
                fb_height,
                option_x,
                option_y,
                option_w,
                option_h,
                selected ? 0.30f : 0.19f,
                selected ? 0.80f : 0.55f,
                selected ? 0.38f : 0.28f);
            draw_text_centered(
                fb_width,
                fb_height,
                option_x,
                option_y,
                option_w,
                option_h,
                option_scale,
                ui_text::quick_menu_option_start(),
                Color {0.96f, 1.00f, 0.96f});
        }
    }

    const MenuStickerRect panel_rect {panel_x, panel_y, panel_w, panel_h};
    const std::array<MenuStickerRect, 3> protected_regions {{
        MenuStickerRect {panel_x + 14.0f, vertical.header_y + 6.0f, panel_w - 28.0f, 46.0f},
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
        MenuStickerSurface::QuickSetup,
        panel_rect,
        protected_regions);
}

}  // namespace whacker::app
