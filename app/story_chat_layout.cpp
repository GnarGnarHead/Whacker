#include "story_chat_layout.hpp"

#include <algorithm>

namespace whacker::app {

StoryChatPortraitLayout make_story_chat_portrait_layout(
    const StoryPanelLayout& panel,
    const StoryPanelLayoutSpec& panel_spec) {
    const float inner_left = panel.panel_x + panel_spec.border_inset_px + 8.0f;
    const float inner_right = panel.panel_x + panel.panel_w - panel_spec.border_inset_px - 8.0f;
    const float inner_width = std::max(0.0f, inner_right - inner_left);

    const float lane_gap = std::clamp(panel.panel_w * 0.008f, 4.0f, 8.0f);
    const float desired_lane_w = std::max(0.0f, panel.panel_h * 1.20f);
    const float min_text_w = std::max(168.0f, panel.panel_w * 0.30f);

    float lane_w = desired_lane_w;
    const float max_lane_w = 0.5f * std::max(0.0f, inner_width - min_text_w - (2.0f * lane_gap));
    lane_w = std::clamp(lane_w, 0.0f, max_lane_w);

    float text_x = inner_left + lane_w + lane_gap;
    float text_right = inner_right - lane_w - lane_gap;
    if (text_right < text_x) {
        text_x = panel.text_x;
        text_right = panel.text_x + panel.text_w;
    }

    const float slot_size = lane_w;
    const float portrait_bottom_anchor_y = panel.panel_y + panel.panel_h - panel_spec.border_inset_px - 2.0f;
    const float portrait_draw_y = portrait_bottom_anchor_y - slot_size;

    StoryChatPortraitLayout layout {};
    layout.text_x = text_x;
    layout.text_w = std::max(0.0f, text_right - text_x);
    layout.left_slot_x = inner_left + std::max(0.0f, 0.5f * (lane_w - slot_size));
    layout.right_slot_x = inner_right - lane_w + std::max(0.0f, 0.5f * (lane_w - slot_size));
    layout.slot_y = portrait_draw_y;
    layout.slot_size = slot_size;
    layout.portrait_size_px = slot_size;
    layout.portrait_draw_y = portrait_draw_y;
    layout.portrait_bottom_anchor_y = portrait_bottom_anchor_y;
    return layout;
}

}  // namespace whacker::app
