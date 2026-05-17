#include "story_overlays.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <string>
#include <string_view>

#include <GLFW/glfw3.h>

#include "menu_sticker_render.hpp"
#include "overlay_layout_math.hpp"
#include "pixel_font.hpp"
#include "story_script_catalog.hpp"
#include "story_panel_layout.hpp"
#include "text_wrap.hpp"
#include "ui_text.hpp"

namespace {

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
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

}  // namespace

namespace whacker::app {

void render_story_menu_overlay(
    GLFWwindow* window,
    const StoryMenuState& menu_state,
    const bool has_save,
    const StoryRowNameFn story_menu_row_name_fn) {
    int fb_width = 0;
    int fb_height = 0;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const auto safe_row_name = [story_menu_row_name_fn](const int row) -> const char* {
        return story_menu_row_name_fn != nullptr ? story_menu_row_name_fn(row) : ui_text::unknown_label();
    };

    StoryPanelLayoutSpec panel_spec {};
    panel_spec.x_fraction = 0.18f;
    panel_spec.y_fraction = 0.18f;
    panel_spec.width_fraction = 0.64f;
    panel_spec.height_fraction = 0.62f;
    panel_spec.header_height_px = 44.0f;
    panel_spec.text_padding_x_px = 16.0f;
    const StoryPanelLayout panel = make_story_panel_layout(fb_width, fb_height, panel_spec);
    draw_story_panel_background(fb_width, fb_height, panel, panel_spec, story_panel_palette());
    const float panel_x = panel.panel_x;
    const float panel_y = panel.panel_y;
    const float panel_w = panel.panel_w;
    const float panel_h = panel.panel_h;
    const MenuStickerRect panel_rect {panel_x, panel_y, panel_w, panel_h};

    constexpr float kTitleScale = 3.0f;
    constexpr float kSubtitleScale = 1.7f;
    constexpr float kFooterScale = 2.0f;
    const OverlayVerticalLayout vertical = make_overlay_vertical_layout(
        panel_y,
        panel_h,
        70.0f,
        text_line_height_pixels(kFooterScale),
        8.0f,
        10.0f,
        8.0f);

    const std::string title = fit_for_width(ui_text::story_menu_title(), panel_w - 32.0f, kTitleScale);
    const std::string subtitle = fit_for_width(ui_text::story_menu_subtitle(), panel_w - 32.0f, kSubtitleScale);
    draw_text_pixels(
        fb_width,
        fb_height,
        panel.text_x,
        vertical.header_y + 8.0f,
        kTitleScale,
        title,
        Color {0.90f, 0.94f, 1.00f});
    draw_text_pixels(
        fb_width,
        fb_height,
        panel.text_x,
        vertical.header_y + 42.0f,
        kSubtitleScale,
        subtitle,
        Color {0.72f, 0.80f, 0.90f});

    const float row_x = panel_x + 20.0f;
    const float row_w = panel_w - 40.0f;
    const OverlayRowLayout rows = make_overlay_row_layout(
        vertical.body_y,
        vertical.body_h,
        StoryMenuRowCount,
        46.0f,
        30.0f,
        10.0f,
        4.0f);
    const float rows_total_h =
        rows.row_h * static_cast<float>(StoryMenuRowCount) +
        rows.row_gap * static_cast<float>(std::max(0, StoryMenuRowCount - 1));
    const float label_scale = rows.row_h < 34.0f ? 1.9f : 2.2f;

    for (int row = 0; row < StoryMenuRowCount; ++row) {
        const bool selected = row == menu_state.selected_row;
        const bool disabled = (row == StoryMenuRowContinue) && !has_save;
        const float y = rows.row_start_y + static_cast<float>(row) * (rows.row_h + rows.row_gap);
        const float base = disabled ? 0.10f : (selected ? 0.20f : 0.12f);
        draw_rect_pixels(
            fb_width,
            fb_height,
            row_x,
            y,
            row_w,
            rows.row_h,
            base,
            disabled ? 0.13f : (selected ? 0.30f : 0.18f),
            disabled ? 0.16f : (selected ? 0.40f : 0.24f));
        draw_text_pixels(
            fb_width,
            fb_height,
            row_x + 10.0f,
            centered_text_y(y, rows.row_h, label_scale),
            label_scale,
            selected ? ui_text::selected_marker() : ui_text::unselected_marker(),
            disabled ? Color {0.44f, 0.48f, 0.54f} : Color {0.96f, 0.86f, 0.34f});

        std::string label = safe_row_name(row);
        if (disabled) {
            label += ui_text::story_menu_no_save_suffix();
        }
        const std::string fitted_label = fit_for_width(label, row_w - 44.0f, label_scale);
        draw_text_pixels(
            fb_width,
            fb_height,
            row_x + 30.0f,
            centered_text_y(y, rows.row_h, label_scale),
            label_scale,
            fitted_label,
            disabled ? Color {0.54f, 0.60f, 0.66f} : Color {0.90f, 0.94f, 1.00f});
    }

    const std::string footer = choose_variant_for_width(
        {ui_text::story_menu_footer(), ui_text::story_menu_footer_short()},
        panel_w - 32.0f,
        kFooterScale);
    draw_text_pixels(
        fb_width,
        fb_height,
        panel.text_x,
        vertical.footer_y,
        kFooterScale,
        footer,
        Color {0.70f, 0.78f, 0.88f});

    const std::array<MenuStickerRect, 3> base_protected_regions {{
        MenuStickerRect {panel.text_x - 2.0f, vertical.header_y + 6.0f, panel.text_w + 4.0f, 58.0f},
        MenuStickerRect {row_x + 20.0f, rows.row_start_y, row_w - 40.0f, rows_total_h},
        MenuStickerRect {
            panel.text_x - 2.0f,
            vertical.footer_y - 4.0f,
            panel.text_w + 4.0f,
            text_line_height_pixels(kFooterScale) + 10.0f,
        },
    }};

    if (!menu_state.confirm_overwrite) {
        render_menu_stickers(
            fb_width,
            fb_height,
            MenuStickerSurface::StoryMenu,
            panel_rect,
            base_protected_regions);
        return;
    }

    const float modal_w = std::max(220.0f, std::min(panel_w * 0.78f, panel_w - 24.0f));
    const float modal_h = std::clamp(panel_h * 0.30f, 96.0f, 140.0f);
    const float modal_x = panel_x + (panel_w - modal_w) * 0.5f;
    const float modal_y = panel_y + panel_h - modal_h - 16.0f;
    draw_rect_pixels(fb_width, fb_height, modal_x, modal_y, modal_w, modal_h, 0.08f, 0.12f, 0.18f);
    draw_rect_pixels(fb_width, fb_height, modal_x + 3.0f, modal_y + 3.0f, modal_w - 6.0f, 26.0f, 0.14f, 0.23f, 0.34f);

    constexpr float kPromptScale = 1.8f;
    constexpr float kHelpScale = 1.6f;
    const std::string prompt = fit_for_width(ui_text::story_menu_overwrite_prompt(), modal_w - 24.0f, kPromptScale);
    const std::string help = choose_variant_for_width(
        {ui_text::story_menu_overwrite_help(), ui_text::story_menu_overwrite_help_short()},
        modal_w - 24.0f,
        kHelpScale);
    draw_text_pixels(
        fb_width,
        fb_height,
        modal_x + 12.0f,
        modal_y + 8.0f,
        kPromptScale,
        prompt,
        Color {0.95f, 0.78f, 0.42f});
    draw_text_pixels(
        fb_width,
        fb_height,
        modal_x + 12.0f,
        modal_y + 34.0f,
        kHelpScale,
        help,
        Color {0.78f, 0.86f, 0.94f});

    const float button_gap = 10.0f;
    const float button_w = std::clamp((modal_w - 24.0f - button_gap) * 0.5f, 88.0f, 120.0f);
    const float button_h = std::clamp(modal_h * 0.28f, 24.0f, 30.0f);
    const float button_y = modal_y + modal_h - button_h - 10.0f;
    const float left_x = modal_x + modal_w - (button_w * 2.0f) - button_gap - 12.0f;
    const float right_x = left_x + button_w + button_gap;
    const bool cancel_selected = menu_state.confirm_selected == 0;
    const bool overwrite_selected = menu_state.confirm_selected == 1;
    const float button_scale = button_h < 26.0f ? 1.4f : 1.6f;

    draw_rect_pixels(
        fb_width,
        fb_height,
        left_x,
        button_y,
        button_w,
        button_h,
        cancel_selected ? 0.26f : 0.16f,
        cancel_selected ? 0.33f : 0.22f,
        cancel_selected ? 0.38f : 0.28f);
    draw_text_centered(
        fb_width,
        fb_height,
        left_x,
        button_y,
        button_w,
        button_h,
        button_scale,
        ui_text::story_menu_cancel_label(),
        Color {0.92f, 0.95f, 0.98f});

    draw_rect_pixels(
        fb_width,
        fb_height,
        right_x,
        button_y,
        button_w,
        button_h,
        overwrite_selected ? 0.42f : 0.22f,
        overwrite_selected ? 0.20f : 0.12f,
        overwrite_selected ? 0.18f : 0.10f);
    draw_text_centered(
        fb_width,
        fb_height,
        right_x,
        button_y,
        button_w,
        button_h,
        button_scale,
        ui_text::story_menu_overwrite_label(),
        Color {0.98f, 0.93f, 0.90f});

    const std::array<MenuStickerRect, 5> protected_regions {{
        base_protected_regions[0],
        base_protected_regions[1],
        base_protected_regions[2],
        MenuStickerRect {modal_x, modal_y, modal_w, modal_h},
        MenuStickerRect {left_x - 2.0f, button_y, (right_x + button_w) - (left_x - 2.0f), button_h},
    }};
    render_menu_stickers(
        fb_width,
        fb_height,
        MenuStickerSurface::StoryMenu,
        panel_rect,
        protected_regions);
}

void render_story_hub_overlay(
    GLFWwindow* window,
    const StoryRuntimeState& story_runtime,
    const StoryHubState& story_hub_state,
    const StoryRowNameFn story_hub_row_name_fn,
    const StoryHubRowEnabledFn story_hub_row_enabled_fn,
    const StorySanitizeNameCallback sanitize_name_fn) {
    int fb_width = 0;
    int fb_height = 0;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const auto safe_row_name = [story_hub_row_name_fn](const int row) -> const char* {
        return story_hub_row_name_fn != nullptr ? story_hub_row_name_fn(row) : ui_text::unknown_label();
    };
    const auto safe_row_enabled =
        [story_hub_row_enabled_fn](const StoryHubRow row, const StoryCareerData& career) -> bool {
            return story_hub_row_enabled_fn != nullptr ? story_hub_row_enabled_fn(row, career) : true;
        };
    const auto safe_sanitize_name = [sanitize_name_fn](const std::string& name) -> std::string {
        return sanitize_name_fn != nullptr ? sanitize_name_fn(name) : name;
    };

    const StoryCareerData& career = story_runtime.career;
    StoryPanelLayoutSpec panel_spec {};
    panel_spec.x_fraction = 0.15f;
    panel_spec.y_fraction = 0.11f;
    panel_spec.width_fraction = 0.70f;
    panel_spec.height_fraction = 0.78f;
    panel_spec.header_height_px = 44.0f;
    panel_spec.text_padding_x_px = 16.0f;
    const StoryPanelLayout panel = make_story_panel_layout(fb_width, fb_height, panel_spec);
    draw_story_panel_background(fb_width, fb_height, panel, panel_spec, story_panel_palette());
    const float panel_x = panel.panel_x;
    const float panel_y = panel.panel_y;
    const float panel_w = panel.panel_w;
    const float panel_h = panel.panel_h;
    const MenuStickerRect panel_rect {panel_x, panel_y, panel_w, panel_h};

    constexpr float kTitleScale = 3.0f;
    constexpr float kInfoScalePrimary = 1.9f;
    constexpr float kInfoScaleSecondary = 1.8f;
    constexpr float kFooterScale = 1.9f;
    const OverlayVerticalLayout vertical = make_overlay_vertical_layout(
        panel_y,
        panel_h,
        126.0f,
        74.0f,
        6.0f,
        8.0f,
        8.0f);

    const float bars_block_w = std::clamp(panel_w * 0.31f, 180.0f, 250.0f);
    const float bars_x = panel_x + panel_w - bars_block_w - 20.0f;
    const float info_w = std::max(120.0f, bars_x - panel.text_x - 10.0f);

    draw_text_pixels(
        fb_width,
        fb_height,
        panel.text_x,
        panel_y + 16.0f,
        kTitleScale,
        fit_for_width(ui_text::story_hub_title(), panel_w - 32.0f, kTitleScale),
        Color {0.90f, 0.94f, 1.00f});
    draw_text_pixels(
        fb_width,
        fb_height,
        panel.text_x,
        panel_y + 52.0f,
        kInfoScalePrimary,
        fit_for_width(
            ui_text::story_hub_player_line(
                safe_sanitize_name(career.player_name),
                career.week,
                static_cast<int>(std::lround(career.reputation.rating))),
            info_w,
            kInfoScalePrimary),
        Color {0.76f, 0.84f, 0.92f});
    draw_text_pixels(
        fb_width,
        fb_height,
        panel.text_x,
        panel_y + 70.0f,
        kInfoScaleSecondary,
        fit_for_width(
            ui_text::story_hub_next_match_line(
                career.official_completed,
                story_graph_has_next_node(career),
                career.training_used),
            info_w,
            kInfoScaleSecondary),
        Color {0.76f, 0.84f, 0.92f});
    draw_text_pixels(
        fb_width,
        fb_height,
        panel.text_x,
        panel_y + 88.0f,
        kInfoScaleSecondary,
        fit_for_width(
            ui_text::story_hub_record_line(career.official_wins, career.official_losses),
            info_w,
            kInfoScaleSecondary),
        Color {0.76f, 0.84f, 0.92f});

    const float bar_w = bars_block_w - 40.0f;
    const float bar_h = 8.0f;
    const float bars_y = panel_y + 52.0f;
    draw_text_pixels(
        fb_width,
        fb_height,
        bars_x,
        bars_y - 2.0f,
        1.5f,
        ui_text::story_hub_power_label(),
        Color {0.95f, 0.52f, 0.36f});
    draw_rect_pixels(fb_width, fb_height, bars_x + 30.0f, bars_y, bar_w, bar_h, 0.13f, 0.17f, 0.22f);
    draw_rect_pixels(
        fb_width,
        fb_height,
        bars_x + 30.0f,
        bars_y,
        bar_w * clampf(career.player_skills.power, 0.0f, 1.0f),
        bar_h,
        0.95f,
        0.52f,
        0.36f);
    draw_text_pixels(
        fb_width,
        fb_height,
        bars_x,
        bars_y + 15.0f,
        1.5f,
        ui_text::story_hub_technical_label(),
        Color {0.42f, 0.89f, 0.56f});
    draw_rect_pixels(fb_width, fb_height, bars_x + 30.0f, bars_y + 17.0f, bar_w, bar_h, 0.13f, 0.17f, 0.22f);
    draw_rect_pixels(
        fb_width,
        fb_height,
        bars_x + 30.0f,
        bars_y + 17.0f,
        bar_w * clampf(career.player_skills.edge, 0.0f, 1.0f),
        bar_h,
        0.42f,
        0.89f,
        0.56f);
    draw_text_pixels(
        fb_width,
        fb_height,
        bars_x,
        bars_y + 32.0f,
        1.5f,
        ui_text::story_hub_spin_label(),
        Color {0.36f, 0.74f, 0.98f});
    draw_rect_pixels(fb_width, fb_height, bars_x + 30.0f, bars_y + 34.0f, bar_w, bar_h, 0.13f, 0.17f, 0.22f);
    draw_rect_pixels(
        fb_width,
        fb_height,
        bars_x + 30.0f,
        bars_y + 34.0f,
        bar_w * clampf(career.player_skills.spin_inject, 0.0f, 1.0f),
        bar_h,
        0.36f,
        0.74f,
        0.98f);

    const float row_x = panel_x + 20.0f;
    const float row_w = panel_w - 40.0f;
    const OverlayRowLayout rows = make_overlay_row_layout(
        vertical.body_y,
        vertical.body_h,
        StoryHubRowCount,
        46.0f,
        30.0f,
        8.0f,
        4.0f);
    const float rows_total_h =
        rows.row_h * static_cast<float>(StoryHubRowCount) +
        rows.row_gap * static_cast<float>(std::max(0, StoryHubRowCount - 1));
    const float row_label_scale = rows.row_h < 34.0f ? 1.9f : 2.2f;
    for (int row = 0; row < StoryHubRowCount; ++row) {
        const bool selected = row == story_hub_state.selected_row;
        const bool enabled = safe_row_enabled(static_cast<StoryHubRow>(row), career);
        const float y = rows.row_start_y + static_cast<float>(row) * (rows.row_h + rows.row_gap);
        draw_rect_pixels(
            fb_width,
            fb_height,
            row_x,
            y,
            row_w,
            rows.row_h,
            enabled ? (selected ? 0.18f : 0.11f) : 0.09f,
            enabled ? (selected ? 0.28f : 0.16f) : 0.11f,
            enabled ? (selected ? 0.40f : 0.22f) : 0.13f);
        draw_text_pixels(
            fb_width,
            fb_height,
            row_x + 8.0f,
            centered_text_y(y, rows.row_h, row_label_scale),
            row_label_scale,
            selected ? ui_text::selected_marker() : ui_text::unselected_marker(),
            enabled ? Color {0.96f, 0.86f, 0.34f} : Color {0.40f, 0.45f, 0.50f});
        draw_text_pixels(
            fb_width,
            fb_height,
            row_x + 28.0f,
            centered_text_y(y, rows.row_h, row_label_scale),
            row_label_scale,
            fit_for_width(safe_row_name(row), row_w - 40.0f, row_label_scale),
            enabled ? Color {0.90f, 0.94f, 1.00f} : Color {0.56f, 0.62f, 0.68f});
    }

    const float feedback_x = panel_x + 16.0f;
    const float feedback_w = panel_w - 32.0f;
    const float feedback_y = panel_y + panel_h - 74.0f;
    draw_rect_pixels(fb_width, fb_height, feedback_x, feedback_y, feedback_w, 46.0f, 0.10f, 0.14f, 0.20f);
    if (!story_hub_state.feedback_line_1.empty()) {
        draw_text_pixels(
            fb_width,
            fb_height,
            feedback_x + 8.0f,
            feedback_y + 12.0f,
            1.8f,
            fit_for_width(story_hub_state.feedback_line_1, feedback_w - 16.0f, 1.8f),
            Color {0.82f, 0.90f, 0.98f});
    }
    if (!story_hub_state.feedback_line_2.empty()) {
        draw_text_pixels(
            fb_width,
            fb_height,
            feedback_x + 8.0f,
            feedback_y + 30.0f,
            1.7f,
            fit_for_width(story_hub_state.feedback_line_2, feedback_w - 16.0f, 1.7f),
            Color {0.70f, 0.82f, 0.92f});
    }

    const std::string footer = choose_variant_for_width(
        {ui_text::story_hub_footer(), ui_text::story_hub_footer_short()},
        panel_w - 32.0f,
        kFooterScale);
    draw_text_pixels(
        fb_width,
        fb_height,
        panel.text_x,
        vertical.footer_y,
        kFooterScale,
        footer,
        Color {0.70f, 0.78f, 0.88f});

    const std::array<MenuStickerRect, 5> protected_regions {{
        MenuStickerRect {panel.text_x - 2.0f, panel_y + 12.0f, panel.text_w + 4.0f, 72.0f},
        MenuStickerRect {bars_x - 2.0f, bars_y - 2.0f, bars_block_w + 4.0f, 52.0f},
        MenuStickerRect {row_x + 20.0f, rows.row_start_y, row_w - 40.0f, rows_total_h},
        MenuStickerRect {feedback_x, feedback_y, feedback_w, 46.0f},
        MenuStickerRect {
            panel.text_x - 2.0f,
            vertical.footer_y - 4.0f,
            panel.text_w + 4.0f,
            text_line_height_pixels(kFooterScale) + 10.0f,
        },
    }};
    render_menu_stickers(
        fb_width,
        fb_height,
        MenuStickerSurface::StoryHub,
        panel_rect,
        protected_regions);
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
