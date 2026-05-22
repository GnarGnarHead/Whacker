#include "story_chat_portrait_overlay.hpp"

#include <algorithm>

#include "story_portrait_render.hpp"

namespace whacker::app {

namespace {

void draw_portrait_slot(
    const int fb_width,
    const int fb_height,
    const float slot_x,
    const float slot_y,
    const float slot_size,
    const StoryPortraitId portrait_id,
    const bool active,
    const bool mirror_x) {
    if (slot_size <= 0.0f || portrait_id == StoryPortraitId::None) {
        return;
    }

    const float inset = std::clamp(slot_size * 0.01f, 0.0f, 1.5f);
    const float portrait_x = slot_x + inset;
    const float portrait_y = slot_y + inset;
    const float portrait_w = std::max(0.0f, slot_size - (2.0f * inset));
    const float portrait_h = std::max(0.0f, slot_size - (2.0f * inset));
    const bool rendered = draw_story_portrait(
        fb_width,
        fb_height,
        portrait_id,
        portrait_x,
        portrait_y,
        portrait_w,
        portrait_h,
        active ? 1.0f : 0.90f,
        active ? 1.08f : 0.90f,
        mirror_x);
    (void)rendered;
}

}  // namespace

void draw_story_chat_portrait_lanes(
    const int fb_width,
    const int fb_height,
    const StoryChatPortraitLayout& layout,
    const StoryPortraitId left_portrait,
    const StoryPortraitId right_portrait,
    const StoryChatPortraitActiveLane active_lane) {
    const float portrait_size = layout.portrait_size_px > 0.0f ? layout.portrait_size_px : layout.slot_size;
    const float portrait_y = layout.portrait_draw_y;
    if (portrait_size <= 0.0f) {
        return;
    }
    draw_portrait_slot(
        fb_width,
        fb_height,
        layout.left_slot_x,
        portrait_y,
        portrait_size,
        left_portrait,
        active_lane == StoryChatPortraitActiveLane::Left,
        true);
    draw_portrait_slot(
        fb_width,
        fb_height,
        layout.right_slot_x,
        portrait_y,
        portrait_size,
        right_portrait,
        active_lane == StoryChatPortraitActiveLane::Right,
        false);
}

}  // namespace whacker::app
