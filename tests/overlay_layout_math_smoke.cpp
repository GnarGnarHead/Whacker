#include <cmath>
#include <cstdlib>

#include "overlay_layout_math.hpp"

namespace {

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

bool nearly_equal(const float a, const float b, const float epsilon = 1.0e-4f) {
    return std::fabs(a - b) <= epsilon;
}

void test_make_overlay_vertical_layout_builds_non_overlapping_regions() {
    const whacker::app::OverlayVerticalLayout layout =
        whacker::app::make_overlay_vertical_layout(100.0f, 300.0f, 64.0f, 16.0f, 8.0f, 10.0f, 6.0f);

    require(nearly_equal(layout.header_y, 108.0f));
    require(nearly_equal(layout.body_y, 178.0f));
    require(nearly_equal(layout.footer_y, 374.0f));
    require(nearly_equal(layout.body_h, 196.0f));
}

void test_make_overlay_row_layout_keeps_rows_inside_body() {
    const float body_y = 80.0f;
    const float body_h = 220.0f;
    const int row_count = 4;
    const whacker::app::OverlayRowLayout layout =
        whacker::app::make_overlay_row_layout(body_y, body_h, row_count, 46.0f, 30.0f, 10.0f, 4.0f);

    const float total_h =
        layout.row_h * static_cast<float>(row_count) + layout.row_gap * static_cast<float>(row_count - 1);
    require(layout.row_h >= 30.0f);
    require(layout.row_gap >= 4.0f);
    require(layout.row_start_y >= body_y);
    require((layout.row_start_y + total_h) <= (body_y + body_h + 1.0e-4f));
}

void test_make_overlay_row_layout_shrinks_when_space_is_tight() {
    const float body_y = 0.0f;
    const float body_h = 80.0f;
    const int row_count = 4;
    const whacker::app::OverlayRowLayout layout =
        whacker::app::make_overlay_row_layout(body_y, body_h, row_count, 40.0f, 24.0f, 8.0f, 2.0f);

    const float total_h =
        layout.row_h * static_cast<float>(row_count) + layout.row_gap * static_cast<float>(row_count - 1);
    require(layout.row_h < 24.0f);
    require(layout.row_h >= 1.0f);
    require(layout.row_gap >= 0.0f);
    require(total_h <= body_h + 1.0e-4f);
}

void test_max_chars_for_text_width_respects_reserved_chars() {
    require(whacker::app::max_chars_for_text_width(160.0f, 2.0f, 0) == 20);
    require(whacker::app::max_chars_for_text_width(160.0f, 2.0f, 3) == 17);
    require(whacker::app::max_chars_for_text_width(0.0f, 2.0f, 0) == 0);
    require(whacker::app::max_chars_for_text_width(160.0f, 0.0f, 0) == 0);
}

void test_inset_text_width_and_safe_budget_reduce_capacity() {
    require(nearly_equal(whacker::app::inset_text_width(200.0f, 8.0f), 184.0f));
    require(nearly_equal(whacker::app::inset_text_width(12.0f, 8.0f), 0.0f));
    require(whacker::app::max_chars_for_safe_text_width(160.0f, 2.0f, 0, 2, 8.0f) == 16);
    require(whacker::app::max_chars_for_safe_text_width(160.0f, 2.0f, 3, 2, 8.0f) == 13);
}

}  // namespace

int main() {
    test_make_overlay_vertical_layout_builds_non_overlapping_regions();
    test_make_overlay_row_layout_keeps_rows_inside_body();
    test_make_overlay_row_layout_shrinks_when_space_is_tight();
    test_max_chars_for_text_width_respects_reserved_chars();
    test_inset_text_width_and_safe_budget_reduce_capacity();
    return 0;
}
