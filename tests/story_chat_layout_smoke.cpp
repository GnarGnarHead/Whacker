#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <utility>

#include "overlay_layout_math.hpp"
#include "story_chat_layout.hpp"
#include "story_panel_layout.hpp"

namespace {

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

float text_char_advance_pixels(const float scale) {
    return 4.0f * scale;
}

float text_line_height_pixels(const float scale) {
    return 5.0f * scale;
}

bool nearly_equal(const float a, const float b, const float epsilon = 1.0e-3f) {
    return std::fabs(a - b) <= epsilon;
}

void test_scene_chat_budget_stays_within_safe_width_for_common_resolutions() {
    constexpr float kGuardPx = 8.0f;
    constexpr int kEarlyWrapChars = 2;
    constexpr float kBodyScale = 2.0f;
    constexpr float kFooterScale = 1.9f;
    constexpr float kLineStep = (5.0f * kBodyScale) + 2.0f;
    const std::array<std::pair<int, int>, 3> kResolutions {{
        {1280, 720},
        {1600, 900},
        {1920, 1080},
    }};

    for (const auto [width, height] : kResolutions) {
        const whacker::app::StoryPanelLayoutSpec spec = whacker::app::story_dialogue_panel_layout_spec();
        const whacker::app::StoryPanelLayout panel = whacker::app::make_story_panel_layout(width, height, spec);
        const whacker::app::StoryChatPortraitLayout chat_layout =
            whacker::app::make_story_chat_portrait_layout(panel, spec);
        const whacker::app::OverlayVerticalLayout vertical = whacker::app::make_overlay_vertical_layout(
            panel.panel_y,
            panel.panel_h,
            40.0f,
            text_line_height_pixels(kFooterScale),
            8.0f,
            10.0f,
            8.0f);

        const float base_text_w = chat_layout.text_w > 0.0f ? chat_layout.text_w : panel.text_w;
        const float safe_width = whacker::app::inset_text_width(base_text_w, kGuardPx);
        const int max_chars = std::max(
            8,
            whacker::app::max_chars_for_safe_text_width(base_text_w, kBodyScale, 0, kEarlyWrapChars, kGuardPx));
        const float max_line_width = static_cast<float>(max_chars) * text_char_advance_pixels(kBodyScale);
        require(max_line_width <= (safe_width + 1.0e-4f));

        const float body_y = vertical.body_y;
        const float body_bottom = vertical.body_y + std::max(0.0f, vertical.body_h - 2.0f);
        const float body_budget = std::max(0.0f, body_bottom - body_y);
        const int max_lines = std::max(1, static_cast<int>(std::floor(body_budget / kLineStep)));
        require(max_lines >= 1);
        require((static_cast<float>(max_lines) * kLineStep) <= (body_budget + kLineStep));
    }
}

void test_intro_option_rows_do_not_overlap_footer_lane() {
    constexpr float kGuardPx = 8.0f;
    constexpr int kEarlyWrapChars = 2;
    constexpr float kFooterScale = 1.9f;
    constexpr float kLine1Scale = 2.2f;
    constexpr float kLine2Scale = 2.1f;
    constexpr float kLine3Scale = 2.2f;
    constexpr int kOptionCount = 2;

    const whacker::app::StoryPanelLayoutSpec spec = whacker::app::story_dialogue_panel_layout_spec();
    const whacker::app::StoryPanelLayout panel = whacker::app::make_story_panel_layout(1280, 720, spec);
    const whacker::app::StoryChatPortraitLayout chat_layout =
        whacker::app::make_story_chat_portrait_layout(panel, spec);
    const whacker::app::OverlayVerticalLayout vertical = whacker::app::make_overlay_vertical_layout(
        panel.panel_y,
        panel.panel_h,
        40.0f,
        text_line_height_pixels(kFooterScale),
        8.0f,
        10.0f,
        8.0f);

    const float base_text_w = chat_layout.text_w > 0.0f ? chat_layout.text_w : panel.text_w;
    const float text_w = whacker::app::inset_text_width(base_text_w, kGuardPx);
    require(std::max(
                4,
                whacker::app::max_chars_for_safe_text_width(text_w, kFooterScale, 0, kEarlyWrapChars, 0.0f)) > 0);

    float cursor_y = vertical.body_y;
    const float body_bottom = vertical.body_y + std::max(0.0f, vertical.body_h - 2.0f);
    if (cursor_y < body_bottom) {
        cursor_y += text_line_height_pixels(kLine1Scale) + 4.0f;
    }
    if (cursor_y < body_bottom) {
        cursor_y += text_line_height_pixels(kLine2Scale) + 4.0f;
    }
    if ((cursor_y + text_line_height_pixels(kLine3Scale)) <= body_bottom) {
        cursor_y += text_line_height_pixels(kLine3Scale) + 4.0f;
    }

    const float option_area_h = std::max(0.0f, body_bottom - cursor_y - 2.0f);
    const whacker::app::OverlayRowLayout option_layout = whacker::app::make_overlay_row_layout(
        cursor_y,
        option_area_h,
        kOptionCount,
        22.0f,
        16.0f,
        4.0f,
        2.0f);
    if (option_area_h <= 0.0f || option_layout.row_h <= 0.0f) {
        return;
    }

    const float option_scale = option_layout.row_h < 20.0f ? 1.8f : 2.0f;
    const float last_option_bottom =
        option_layout.row_start_y +
        static_cast<float>(kOptionCount - 1) * (option_layout.row_h + option_layout.row_gap) +
        text_line_height_pixels(option_scale);
    require(last_option_bottom <= (body_bottom + 1.0e-3f));
}

void test_portrait_slots_and_text_column_stay_inside_panel_bounds() {
    const whacker::app::StoryPanelLayoutSpec spec = whacker::app::story_dialogue_panel_layout_spec();
    const std::array<std::pair<int, int>, 4> kResolutions {{
        {960, 540},
        {1280, 720},
        {1600, 900},
        {1920, 1080},
    }};

    for (const auto [width, height] : kResolutions) {
        const whacker::app::StoryPanelLayout panel = whacker::app::make_story_panel_layout(width, height, spec);
        const whacker::app::StoryChatPortraitLayout chat_layout =
            whacker::app::make_story_chat_portrait_layout(panel, spec);

        const float inner_left = panel.panel_x + spec.border_inset_px + 8.0f;
        const float inner_right = panel.panel_x + panel.panel_w - spec.border_inset_px - 8.0f;
        const float inner_width = std::max(0.0f, inner_right - inner_left);
        const float lane_gap = std::clamp(panel.panel_w * 0.008f, 4.0f, 8.0f);
        const float desired_lane_w = std::max(0.0f, panel.panel_h * 1.20f);
        const float min_text_w = std::max(168.0f, panel.panel_w * 0.30f);
        const float max_lane_w = 0.5f * std::max(0.0f, inner_width - min_text_w - (2.0f * lane_gap));
        const float expected_lane_w = std::clamp(desired_lane_w, 0.0f, max_lane_w);
        const float expected_bottom_anchor = panel.panel_y + panel.panel_h - spec.border_inset_px - 2.0f;

        require(chat_layout.text_w > 0.0f);
        require(chat_layout.text_x >= panel.panel_x);
        require((chat_layout.text_x + chat_layout.text_w) <= (panel.panel_x + panel.panel_w + 1.0e-3f));
        require(nearly_equal(chat_layout.portrait_size_px, expected_lane_w));
        require(nearly_equal(chat_layout.slot_size, expected_lane_w));
        require(nearly_equal(chat_layout.portrait_bottom_anchor_y, expected_bottom_anchor));
        require(nearly_equal(chat_layout.portrait_draw_y + chat_layout.portrait_size_px, expected_bottom_anchor));
        require(nearly_equal(chat_layout.slot_y, chat_layout.portrait_draw_y));

        if (chat_layout.slot_size <= 0.0f) {
            continue;
        }

        const float left_slot_right = chat_layout.left_slot_x + chat_layout.slot_size;
        const float right_slot_right = chat_layout.right_slot_x + chat_layout.slot_size;
        const float slot_bottom = chat_layout.portrait_draw_y + chat_layout.portrait_size_px;
        require(chat_layout.left_slot_x >= panel.panel_x);
        require(right_slot_right <= (panel.panel_x + panel.panel_w + 1.0e-3f));
        require(left_slot_right <= chat_layout.text_x);
        require(chat_layout.right_slot_x >= (chat_layout.text_x + chat_layout.text_w));
        require(slot_bottom <= (panel.panel_y + panel.panel_h + 1.0e-3f));
    }
}

void test_portrait_slots_follow_panel_height_scaling() {
    const whacker::app::StoryPanelLayoutSpec spec = whacker::app::story_dialogue_panel_layout_spec();

    const whacker::app::StoryPanelLayout panel_720 = whacker::app::make_story_panel_layout(1280, 720, spec);
    const whacker::app::StoryChatPortraitLayout layout_720 =
        whacker::app::make_story_chat_portrait_layout(panel_720, spec);
    require(layout_720.slot_size >= 112.0f);

    const whacker::app::StoryPanelLayout panel_1080 = whacker::app::make_story_panel_layout(1920, 1080, spec);
    const whacker::app::StoryChatPortraitLayout layout_1080 =
        whacker::app::make_story_chat_portrait_layout(panel_1080, spec);

    const float expected_720 = panel_720.panel_h * 1.20f;
    const float expected_1080 = panel_1080.panel_h * 1.20f;
    require(nearly_equal(layout_720.portrait_size_px, expected_720));
    require(nearly_equal(layout_1080.portrait_size_px, expected_1080));

    const float actual_ratio = layout_1080.portrait_size_px / layout_720.portrait_size_px;
    const float expected_ratio = expected_1080 / expected_720;
    require(nearly_equal(actual_ratio, expected_ratio, 1.0e-2f));

    require(layout_720.portrait_size_px >= 220.0f);
    require(layout_1080.portrait_size_px >= 330.0f);
}

}  // namespace

int main() {
    test_scene_chat_budget_stays_within_safe_width_for_common_resolutions();
    test_intro_option_rows_do_not_overlap_footer_lane();
    test_portrait_slots_and_text_column_stay_inside_panel_bounds();
    test_portrait_slots_follow_panel_height_scaling();
    return 0;
}
