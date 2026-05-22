#pragma once

#include <string>
#include <vector>

#include "app_types.hpp"

namespace whacker::app {

struct ControlHintBindings;
struct StoryIntroState;
using StoryIntroSanitizeNameFn = std::string (*)(const std::string&);
using StoryIntroKeyNameFn = const char* (*)(int);

struct StoryIntroBodyRow {
    std::string text {};
    float scale = 2.0f;
    Color color {};
    float advance_px = 0.0f;
};

struct StoryIntroBodyLayout {
    std::vector<StoryIntroBodyRow> rows {};
    float body_budget_px = 0.0f;
    int latest_start_row = 0;
    int max_scroll_rows = 0;
};

StoryIntroBodyLayout compute_story_intro_body_layout_for_framebuffer(
    int fb_width,
    int fb_height,
    const StoryIntroState& story_intro_state,
    const ControlHintBindings& controls,
    StoryIntroKeyNameFn key_name_fn,
    StoryIntroSanitizeNameFn sanitize_name_fn);

int clamp_story_intro_scroll_from_bottom(
    const StoryIntroBodyLayout& layout,
    int requested_scroll_from_bottom);

int first_visible_story_intro_row_index(
    const StoryIntroBodyLayout& layout,
    int scroll_from_bottom);

}  // namespace whacker::app
