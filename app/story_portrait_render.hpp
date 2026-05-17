#pragma once

#include "story_portraits.hpp"

#ifdef WHACKER_HAS_GLFW

namespace whacker::app {

bool draw_story_portrait(
    int fb_width,
    int fb_height,
    StoryPortraitId portrait_id,
    float x,
    float y,
    float w,
    float h,
    float alpha = 1.0f,
    float brightness = 1.0f,
    bool mirror_x = false);

void release_story_portrait_resources();

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
