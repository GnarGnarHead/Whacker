#pragma once

#include "action_input.hpp"
#include "ui_state.hpp"

namespace whacker::app {

enum class StoryMenuActionResult {
    None,
    Back,
    Continue,
    NewCareer
};

StoryMenuActionResult apply_story_menu_action(
    StoryMenuState& story_menu_state,
    bool has_save,
    bool move_up,
    bool move_down,
    bool move_left,
    bool move_right,
    bool confirm,
    bool back);

StoryMenuActionResult apply_story_menu_action_frame(
    StoryMenuState& story_menu_state,
    bool has_save,
    const ActionInputFrame& input);

}  // namespace whacker::app
