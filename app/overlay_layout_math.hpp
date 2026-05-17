#pragma once

namespace whacker::app {

struct OverlayVerticalLayout {
    float header_y = 0.0f;
    float body_y = 0.0f;
    float body_h = 0.0f;
    float footer_y = 0.0f;
};

struct OverlayRowLayout {
    float row_start_y = 0.0f;
    float row_h = 0.0f;
    float row_gap = 0.0f;
};

OverlayVerticalLayout make_overlay_vertical_layout(
    float panel_y,
    float panel_h,
    float header_h,
    float footer_h,
    float top_padding,
    float bottom_padding,
    float section_gap);

OverlayRowLayout make_overlay_row_layout(
    float body_y,
    float body_h,
    int row_count,
    float preferred_row_h,
    float min_row_h,
    float preferred_gap,
    float min_gap);

int max_chars_for_text_width(float width_px, float scale, int reserved_chars = 0);

float inset_text_width(float width_px, float inner_guard_px);

int max_chars_for_safe_text_width(
    float width_px,
    float scale,
    int reserved_chars,
    int early_wrap_chars,
    float inner_guard_px);

}  // namespace whacker::app
