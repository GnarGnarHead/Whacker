#include "story_menu_actions.hpp"

namespace whacker::app {

namespace {

void clear_overwrite_confirm(StoryMenuState& story_menu_state) {
    story_menu_state.confirm_overwrite = false;
    story_menu_state.confirm_selected = 0;
}

}  // namespace

StoryMenuActionResult apply_story_menu_action(
    StoryMenuState& story_menu_state,
    const bool has_save,
    const MenuIntent& intent) {
    if (story_menu_state.confirm_overwrite) {
        if (intent.back) {
            clear_overwrite_confirm(story_menu_state);
            return StoryMenuActionResult::None;
        }
        if (intent.up || intent.down || intent.left || intent.right) {
            story_menu_state.confirm_selected = 1 - story_menu_state.confirm_selected;
        }
        if (!intent.confirm) {
            return StoryMenuActionResult::None;
        }
        if (story_menu_state.confirm_selected == 1) {
            clear_overwrite_confirm(story_menu_state);
            return StoryMenuActionResult::NewCareer;
        }
        clear_overwrite_confirm(story_menu_state);
        return StoryMenuActionResult::None;
    }

    if (intent.up) {
        story_menu_state.selected_row = (story_menu_state.selected_row + StoryMenuRowCount - 1) % StoryMenuRowCount;
    }
    if (intent.down) {
        story_menu_state.selected_row = (story_menu_state.selected_row + 1) % StoryMenuRowCount;
    }
    if (intent.back) {
        story_menu_state.selected_row = StoryMenuRowBack;
        return StoryMenuActionResult::Back;
    }
    if (!intent.confirm) {
        return StoryMenuActionResult::None;
    }

    if (story_menu_state.selected_row == StoryMenuRowBack) {
        return StoryMenuActionResult::Back;
    }
    if (story_menu_state.selected_row == StoryMenuRowContinue) {
        return has_save ? StoryMenuActionResult::Continue : StoryMenuActionResult::None;
    }
    if (story_menu_state.selected_row == StoryMenuRowNewCareer) {
        if (has_save) {
            story_menu_state.confirm_overwrite = true;
            story_menu_state.confirm_selected = 0;
            return StoryMenuActionResult::None;
        }
        return StoryMenuActionResult::NewCareer;
    }
    return StoryMenuActionResult::None;
}

}  // namespace whacker::app
