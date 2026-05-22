#include "overlay_layout_math.hpp"

#include <algorithm>
#include <cmath>

#include "pixel_font.hpp"

namespace whacker::app {

namespace {

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

}  // namespace

OverlayVerticalLayout make_overlay_vertical_layout(
    const float panel_y,
    const float panel_h,
    const float header_h,
    const float footer_h,
    const float top_padding,
    const float bottom_padding,
    const float section_gap) {
    const float safe_panel_h = std::max(0.0f, panel_h);
    const float safe_header_h = std::max(0.0f, header_h);
    const float safe_footer_h = std::max(0.0f, footer_h);
    const float safe_top_padding = std::max(0.0f, top_padding);
    const float safe_bottom_padding = std::max(0.0f, bottom_padding);
    const float safe_section_gap = std::max(0.0f, section_gap);

    OverlayVerticalLayout layout {};
    layout.header_y = panel_y + safe_top_padding;
    layout.body_y = layout.header_y + safe_header_h + safe_section_gap;
    layout.footer_y = panel_y + safe_panel_h - safe_bottom_padding - safe_footer_h;
    if (layout.footer_y < layout.body_y) {
        layout.footer_y = layout.body_y;
    }
    layout.body_h = std::max(0.0f, layout.footer_y - layout.body_y);
    return layout;
}

OverlayRowLayout make_overlay_row_layout(
    const float body_y,
    const float body_h,
    const int row_count,
    const float preferred_row_h,
    const float min_row_h,
    const float preferred_gap,
    const float min_gap) {
    OverlayRowLayout layout {};
    layout.row_start_y = body_y;
    if (row_count <= 0 || body_h <= 0.0f) {
        return layout;
    }

    const int rows = std::max(1, row_count);
    const float safe_body_h = std::max(0.0f, body_h);
    const float safe_min_row_h = std::max(8.0f, min_row_h);
    const float safe_pref_row_h = std::max(safe_min_row_h, preferred_row_h);
    const float safe_pref_gap = std::max(0.0f, preferred_gap);
    const float safe_min_gap = clampf(min_gap, 0.0f, safe_pref_gap);

    float row_h = safe_pref_row_h;
    float row_gap = rows > 1 ? safe_pref_gap : 0.0f;
    float used_h = row_h * static_cast<float>(rows) + row_gap * static_cast<float>(rows - 1);

    if (used_h > safe_body_h) {
        if (rows > 1) {
            const float max_gap_that_fits =
                std::max(0.0f, (safe_body_h - (row_h * static_cast<float>(rows))) / static_cast<float>(rows - 1));
            row_gap = clampf(max_gap_that_fits, safe_min_gap, safe_pref_gap);
        } else {
            row_gap = 0.0f;
        }
        used_h = row_h * static_cast<float>(rows) + row_gap * static_cast<float>(rows - 1);
    }

    if (used_h > safe_body_h) {
        const float baseline_gap = rows > 1 ? safe_min_gap : 0.0f;
        row_h = (safe_body_h - (baseline_gap * static_cast<float>(rows - 1))) / static_cast<float>(rows);
        row_h = std::min(safe_pref_row_h, row_h);
        if (row_h < 1.0f) {
            row_h = 1.0f;
        }
        if (rows > 1) {
            row_gap = std::max(
                0.0f,
                (safe_body_h - (row_h * static_cast<float>(rows))) / static_cast<float>(rows - 1));
        } else {
            row_gap = 0.0f;
        }
        used_h = row_h * static_cast<float>(rows) + row_gap * static_cast<float>(rows - 1);
    }

    layout.row_h = row_h;
    layout.row_gap = row_gap;
    layout.row_start_y = body_y + std::max(0.0f, 0.5f * (safe_body_h - used_h));
    return layout;
}

int max_chars_for_text_width(const float width_px, const float scale, const int reserved_chars) {
    if (width_px <= 0.0f || scale <= 0.0f) {
        return 0;
    }
    const float advance_px = 4.0f * pixel_font_render_scale(scale);
    if (advance_px <= 0.0f) {
        return 0;
    }
    const int max_chars = static_cast<int>(std::floor(width_px / advance_px));
    return std::max(0, max_chars - std::max(0, reserved_chars));
}

float inset_text_width(const float width_px, const float inner_guard_px) {
    const float safe_width = std::max(0.0f, width_px);
    const float safe_guard = std::max(0.0f, inner_guard_px);
    return std::max(0.0f, safe_width - (2.0f * safe_guard));
}

int max_chars_for_safe_text_width(
    const float width_px,
    const float scale,
    const int reserved_chars,
    const int early_wrap_chars,
    const float inner_guard_px) {
    const int base_chars = max_chars_for_text_width(inset_text_width(width_px, inner_guard_px), scale, reserved_chars);
    return std::max(0, base_chars - std::max(0, early_wrap_chars));
}

}  // namespace whacker::app
