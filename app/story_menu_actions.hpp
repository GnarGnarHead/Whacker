#pragma once

#include "menu_intent.hpp"
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
    const MenuIntent& intent);

}  // namespace whacker::app
