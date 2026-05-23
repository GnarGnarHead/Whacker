#include "menu_overlay.hpp"

#include <algorithm>
#include <array>
#include <initializer_list>
#include <string>
#include <string_view>

#include "menu_sticker_render.hpp"
#include "overlay_layout_math.hpp"
#include "options_menu_actions.hpp"
#include "pixel_font.hpp"
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

float fit_scale_for_width(
    const std::string_view text,
    const float width_px,
    const float desired_scale,
    const float min_scale) {
    if (text.empty() || width_px <= 0.0f || desired_scale <= 0.0f) {
        return desired_scale;
    }
    const float max_scale = width_px / std::max(1.0f, static_cast<float>(text.size()) * 4.0f);
    return std::clamp(max_scale, std::min(min_scale, desired_scale), desired_scale);
}

float fit_scale_for_box(
    const std::string_view text,
    const float width_px,
    const float height_px,
    const float desired_scale,
    const float min_scale) {
    const float width_scale = fit_scale_for_width(text, width_px, desired_scale, min_scale);
    const float height_scale = std::max(0.1f, (height_px - 4.0f) / 5.0f);
    return std::min(width_scale, height_scale);
}

}  // namespace

void render_main_menu_overlay(
    const RenderContext& context,
    const MainMenuState& menu_state,
    const RowNameFn row_name_fn,
    const std::string& status_message) {
    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
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

    const bool has_status = !status_message.empty();
    const std::string footer = has_status
        ? fit_for_width(status_message, panel_w - 32.0f, kFooterScale)
        : choose_variant_for_width(
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
        has_status ? Color {0.96f, 0.86f, 0.34f} : Color {0.70f, 0.78f, 0.88f});

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

void render_options_menu_overlay(
    const RenderContext& context,
    const OptionsMenuState& menu_state,
    const OptionsRowNameFn row_name_fn,
    const OptionsValueLabelFn value_label_fn,
    const void* value_label_context) {
    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const auto safe_row_name = [row_name_fn, &menu_state](const int row) -> const char* {
        return row_name_fn != nullptr ? row_name_fn(menu_state.section, row) : ui_text::unknown_label();
    };
    const auto safe_value_label =
        [value_label_fn, value_label_context, &menu_state](const int row) -> std::string {
            return value_label_fn != nullptr ? value_label_fn(menu_state, row, value_label_context) : std::string {};
        };

    const float panel_x = static_cast<float>(fb_width) * 0.16f;
    const float panel_y = static_cast<float>(fb_height) * 0.12f;
    const float panel_w = static_cast<float>(fb_width) * 0.68f;
    const float panel_h = static_cast<float>(fb_height) * 0.76f;
    draw_rect_pixels(fb_width, fb_height, panel_x, panel_y, panel_w, panel_h, 0.05f, 0.09f, 0.14f);
    draw_rect_pixels(fb_width, fb_height, panel_x + 4.0f, panel_y + 4.0f, panel_w - 8.0f, 44.0f, 0.09f, 0.16f, 0.24f);

    constexpr float kOptionsReadabilityScale = 1.12f;
    constexpr float kTitleScale = 3.0f * kOptionsReadabilityScale;
    constexpr float kSubtitleScale = 1.8f * kOptionsReadabilityScale;
    constexpr float kFooterScale = 1.6f * kOptionsReadabilityScale;
    const OverlayVerticalLayout vertical = make_overlay_vertical_layout(
        panel_y,
        panel_h,
        72.0f,
        text_line_height_pixels(kFooterScale),
        8.0f,
        10.0f,
        8.0f);

    const char* raw_title = ui_text::options_title();
    const char* raw_subtitle = ui_text::options_subtitle();
    if (menu_state.section == OptionsMenuSection::Controls) {
        raw_title = ui_text::options_controls_title();
        raw_subtitle = ui_text::options_controls_subtitle();
    } else if (menu_state.section == OptionsMenuSection::Audio) {
        raw_title = ui_text::options_audio_title();
        raw_subtitle = ui_text::options_audio_subtitle();
    }
    const std::string title = fit_for_width(raw_title, panel_w - 32.0f, kTitleScale);
    const std::string subtitle = fit_for_width(raw_subtitle, panel_w - 32.0f, kSubtitleScale);
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

    const float row_x = panel_x + 20.0f;
    const float row_w = panel_w - 40.0f;
    const int row_count = options_menu_row_count(menu_state.section);
    const OverlayRowLayout rows = make_overlay_row_layout(
        vertical.body_y,
        vertical.body_h,
        row_count,
        42.0f,
        28.0f,
        6.0f,
        3.0f);
    const float rows_total_h =
        rows.row_h * static_cast<float>(row_count) +
        rows.row_gap * static_cast<float>(std::max(0, row_count - 1));
    const float value_w = std::clamp(row_w * 0.46f, 200.0f, 380.0f);

    for (int row = 0; row < row_count; ++row) {
        const bool selected = row == menu_state.selected_row;
        const float y = rows.row_start_y + static_cast<float>(row) * (rows.row_h + rows.row_gap);
        const float label_scale = (rows.row_h < 36.0f ? 1.8f : 2.0f) * kOptionsReadabilityScale;
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
            centered_text_y(y, rows.row_h, label_scale),
            label_scale,
            selected ? ui_text::selected_marker() : ui_text::unselected_marker(),
            Color {0.96f, 0.86f, 0.34f});

        const bool is_back_row = options_row_is_back(menu_state.section, row);
        const bool binding_row = options_row_is_binding(menu_state.section, row);
        const bool axis_invert_row = options_row_is_axis_invert(menu_state.section, row);
        const bool control_preset_row = options_row_is_control_preset(menu_state.section, row);
        const bool volume_row = options_row_is_volume(menu_state.section, row);
        const bool mute_row = options_row_is_mute(menu_state.section, row);
        const bool has_value = control_preset_row || binding_row || axis_invert_row || volume_row || mute_row;
        const float label_w = (is_back_row || !has_value) ? (row_w - 34.0f) : (row_w - value_w - 48.0f);
        const std::string label = fit_for_width(safe_row_name(row), label_w, label_scale);
        draw_text_pixels(
            fb_width,
            fb_height,
            row_x + 30.0f,
            centered_text_y(y, rows.row_h, label_scale),
            label_scale,
            label,
            Color {0.90f, 0.94f, 1.00f});

        if (is_back_row || !has_value) {
            continue;
        }

        const float value_h = std::max(0.0f, std::min(rows.row_h - 4.0f, 34.0f));
        const float value_x = row_x + row_w - value_w - 14.0f;
        const float value_y = y + std::max(0.0f, 0.5f * (rows.row_h - value_h));
        const bool waiting_on_row = binding_row && menu_state.waiting_for_input && selected;
        draw_rect_pixels(
            fb_width,
            fb_height,
            value_x,
            value_y,
            value_w,
            value_h,
            waiting_on_row ? 0.36f : 0.18f,
            waiting_on_row ? 0.25f : 0.26f,
            waiting_on_row ? 0.16f : 0.34f);
        std::string value_label;
        if (waiting_on_row) {
            value_label = ui_text::options_waiting_value();
        } else if (control_preset_row || binding_row || axis_invert_row || volume_row || mute_row) {
            value_label = safe_value_label(row);
        }
        const float desired_value_scale = menu_state.section == OptionsMenuSection::Controls ? 2.25f : 2.35f;
        const float value_scale = fit_scale_for_box(value_label, value_w - 16.0f, value_h, desired_value_scale, 1.75f);
        const std::string fitted_value = fit_for_width(value_label, value_w - 16.0f, value_scale);
        draw_text_centered(
            fb_width,
            fb_height,
            value_x,
            value_y,
            value_w,
            value_h,
            value_scale,
            fitted_value,
            Color {0.94f, 0.97f, 1.00f});
    }

    const std::string footer = menu_state.waiting_for_input
        ? choose_variant_for_width(
            {ui_text::options_footer_waiting_for_key(), ui_text::options_footer_waiting_for_key_short()},
            panel_w - 32.0f,
            kFooterScale)
        : menu_state.section == OptionsMenuSection::Root
            ? choose_variant_for_width(
                {ui_text::options_footer_root(), ui_text::options_footer_root_short()},
                panel_w - 32.0f,
                kFooterScale)
            : choose_variant_for_width(
                {ui_text::options_footer_default(), ui_text::options_footer_default_short()},
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

    const MenuStickerRect panel_rect {panel_x, panel_y, panel_w, panel_h};
    const std::array<MenuStickerRect, 3> protected_regions {{
        MenuStickerRect {panel_x + 14.0f, vertical.header_y + 6.0f, panel_w - 28.0f, 58.0f},
        MenuStickerRect {row_x + 18.0f, rows.row_start_y, row_w - 36.0f, rows_total_h},
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
        MenuStickerSurface::OptionsMenu,
        panel_rect,
        protected_regions);
}

void render_pause_overlay(
    const RenderContext& context,
    const PauseMenuState& pause_menu_state,
    const MatchExitPolicy& exit_policy) {
    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const float panel_w = std::max(220.0f, std::min(460.0f, static_cast<float>(fb_width) - 24.0f));
    const float panel_h = std::max(170.0f, std::min(250.0f, static_cast<float>(fb_height) - 24.0f));
    const float panel_x = 0.5f * (static_cast<float>(fb_width) - panel_w);
    const float panel_y = 0.5f * (static_cast<float>(fb_height) - panel_h);
    const MenuStickerRect panel_rect {panel_x, panel_y, panel_w, panel_h};

    draw_rect_pixels(fb_width, fb_height, panel_x, panel_y, panel_w, panel_h, 0.05f, 0.09f, 0.14f);
    draw_rect_pixels(fb_width, fb_height, panel_x + 4.0f, panel_y + 4.0f, panel_w - 8.0f, 44.0f, 0.09f, 0.16f, 0.24f);
    constexpr float kTitleScale = 2.8f;
    constexpr float kFooterScale = 1.9f;
    const OverlayVerticalLayout vertical = make_overlay_vertical_layout(
        panel_y,
        panel_h,
        56.0f,
        text_line_height_pixels(kFooterScale),
        8.0f,
        10.0f,
        8.0f);
    const std::string title = fit_for_width(ui_text::pause_title(), panel_w - 32.0f, kTitleScale);
    draw_text_pixels(
        fb_width,
        fb_height,
        panel_x + 16.0f,
        vertical.header_y + 8.0f,
        kTitleScale,
        title,
        Color {0.90f, 0.94f, 1.00f});

    const float row_x = panel_x + 18.0f;
    const float row_w = panel_w - 36.0f;

    if (!pause_menu_state.confirm_forfeit) {
        const std::string exit_label = exit_policy.exit_label.empty()
            ? std::string(ui_text::pause_default_exit_label())
            : std::string(exit_policy.exit_label);
        const bool has_forfeit_row = exit_policy.has_exit_option;
        const int visible_rows = has_forfeit_row ? PauseMenuRowCount : (PauseMenuRowCount - 1);
        const OverlayRowLayout rows = make_overlay_row_layout(
            vertical.body_y,
            vertical.body_h,
            visible_rows,
            52.0f,
            34.0f,
            12.0f,
            6.0f);
        const float rows_total_h =
            rows.row_h * static_cast<float>(visible_rows) +
            rows.row_gap * static_cast<float>(std::max(0, visible_rows - 1));
        for (int row = 0; row < visible_rows; ++row) {
            const bool selected = row == pause_menu_state.selected_row;
            const bool is_resume_row = row == PauseMenuRowResume;
            const bool is_forfeit_row = has_forfeit_row && row == PauseMenuRowExitMatch;
            const char* row_label = is_resume_row
                ? ui_text::pause_resume_label()
                : (is_forfeit_row ? exit_label.c_str() : ui_text::pause_quit_label());
            const float y = rows.row_start_y + static_cast<float>(row) * (rows.row_h + rows.row_gap);
            const float label_scale = rows.row_h < 40.0f ? 1.9f : 2.3f;
            const std::string fitted_label = fit_for_width(row_label, row_w - 44.0f, label_scale);
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
                centered_text_y(y, rows.row_h, label_scale),
                label_scale,
                selected ? ui_text::selected_marker() : ui_text::unselected_marker(),
                Color {0.96f, 0.86f, 0.34f});
            draw_text_pixels(
                fb_width,
                fb_height,
                row_x + 30.0f,
                centered_text_y(y, rows.row_h, label_scale),
                label_scale,
                fitted_label,
                is_forfeit_row && !exit_policy.can_exit_now
                    ? Color {0.64f, 0.70f, 0.78f}
                    : Color {0.90f, 0.94f, 1.00f});
        }

        const std::string footer = choose_variant_for_width(
            {ui_text::pause_footer(exit_policy.blocked_reason), ui_text::pause_footer_short(exit_policy.blocked_reason)},
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
        const std::array<MenuStickerRect, 3> protected_regions {{
            MenuStickerRect {panel_x + 14.0f, vertical.header_y + 6.0f, panel_w - 28.0f, 44.0f},
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
            MenuStickerSurface::PauseMenu,
            panel_rect,
            protected_regions);
        return;
    }

    constexpr float kPromptScale = 2.4f;
    const std::string prompt = fit_for_width(ui_text::pause_forfeit_prompt(), panel_w - 48.0f, kPromptScale);
    draw_text_pixels(
        fb_width,
        fb_height,
        panel_x + 24.0f,
        vertical.body_y,
        kPromptScale,
        prompt,
        Color {0.95f, 0.88f, 0.46f});

    const float confirm_h = std::clamp(vertical.body_h - text_line_height_pixels(kPromptScale) - 12.0f, 34.0f, 52.0f);
    const float confirm_gap = 24.0f;
    const float confirm_w = std::clamp((panel_w - 56.0f - confirm_gap) * 0.5f, 90.0f, 180.0f);
    const float confirm_y = vertical.body_y + text_line_height_pixels(kPromptScale) + 10.0f;
    const float no_x = panel_x + 0.5f * (panel_w - ((confirm_w * 2.0f) + confirm_gap));
    const float yes_x = no_x + confirm_w + confirm_gap;
    const bool no_selected = pause_menu_state.confirm_selected == 0;
    const bool yes_selected = pause_menu_state.confirm_selected == 1;

    draw_rect_pixels(
        fb_width,
        fb_height,
        no_x,
        confirm_y,
        confirm_w,
        confirm_h,
        no_selected ? 0.20f : 0.12f,
        no_selected ? 0.30f : 0.18f,
        no_selected ? 0.40f : 0.24f);
    draw_rect_pixels(
        fb_width,
        fb_height,
        yes_x,
        confirm_y,
        confirm_w,
        confirm_h,
        yes_selected ? 0.36f : 0.14f,
        yes_selected ? 0.19f : 0.18f,
        yes_selected ? 0.12f : 0.24f);
    draw_text_centered(
        fb_width,
        fb_height,
        no_x,
        confirm_y,
        confirm_w,
        confirm_h,
        confirm_h < 44.0f ? 1.8f : 2.2f,
        ui_text::selected_option_label(ui_text::no_label(), no_selected),
        Color {0.90f, 0.94f, 1.00f});
    draw_text_centered(
        fb_width,
        fb_height,
        yes_x,
        confirm_y,
        confirm_w,
        confirm_h,
        confirm_h < 44.0f ? 1.8f : 2.2f,
        ui_text::selected_option_label(ui_text::yes_label(), yes_selected),
        Color {0.96f, 0.86f, 0.34f});

    const std::string footer = choose_variant_for_width(
        {ui_text::pause_confirm_footer(), ui_text::pause_confirm_footer_short()},
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

    const std::array<MenuStickerRect, 4> protected_regions {{
        MenuStickerRect {panel_x + 14.0f, vertical.header_y + 6.0f, panel_w - 28.0f, 44.0f},
        MenuStickerRect {
            panel_x + 24.0f,
            vertical.body_y,
            panel_w - 48.0f,
            text_line_height_pixels(kPromptScale),
        },
        MenuStickerRect {
            no_x - 2.0f,
            confirm_y,
            (yes_x + confirm_w) - (no_x - 2.0f),
            confirm_h,
        },
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
        MenuStickerSurface::PauseMenu,
        panel_rect,
        protected_regions);
}

}  // namespace whacker::app
