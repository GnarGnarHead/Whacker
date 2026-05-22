#pragma once

#include <cstdint>

#include "story_chat_layout.hpp"
#include "story_portraits.hpp"

namespace whacker::app {

enum class StoryChatPortraitActiveLane : std::uint8_t {
    None = 0,
    Left = 1,
    Right = 2,
};

void draw_story_chat_portrait_lanes(
    int fb_width,
    int fb_height,
    const StoryChatPortraitLayout& layout,
    StoryPortraitId left_portrait,
    StoryPortraitId right_portrait,
    StoryChatPortraitActiveLane active_lane);

}  // namespace whacker::app
