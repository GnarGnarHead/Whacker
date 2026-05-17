#pragma once

#include "story_panel_layout.hpp"

namespace whacker::app {

struct StoryChatPortraitLayout {
    float text_x = 0.0f;
    float text_w = 0.0f;
    float left_slot_x = 0.0f;
    float right_slot_x = 0.0f;
    float slot_y = 0.0f;
    float slot_size = 0.0f;
    float portrait_size_px = 0.0f;
    float portrait_draw_y = 0.0f;
    float portrait_bottom_anchor_y = 0.0f;
};

StoryChatPortraitLayout make_story_chat_portrait_layout(
    const StoryPanelLayout& panel,
    const StoryPanelLayoutSpec& panel_spec);

}  // namespace whacker::app
