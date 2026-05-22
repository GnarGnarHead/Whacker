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
    const bool move_up,
    const bool move_down,
    const bool move_left,
    const bool move_right,
    const bool confirm,
    const bool back) {
    if (story_menu_state.confirm_overwrite) {
        if (back) {
            clear_overwrite_confirm(story_menu_state);
            return StoryMenuActionResult::None;
        }
        if (move_up || move_down || move_left || move_right) {
            story_menu_state.confirm_selected = 1 - story_menu_state.confirm_selected;
        }
        if (!confirm) {
            return StoryMenuActionResult::None;
        }
        if (story_menu_state.confirm_selected == 1) {
            clear_overwrite_confirm(story_menu_state);
            return StoryMenuActionResult::NewCareer;
        }
        clear_overwrite_confirm(story_menu_state);
        return StoryMenuActionResult::None;
    }

    if (move_up) {
        story_menu_state.selected_row = (story_menu_state.selected_row + StoryMenuRowCount - 1) % StoryMenuRowCount;
    }
    if (move_down) {
        story_menu_state.selected_row = (story_menu_state.selected_row + 1) % StoryMenuRowCount;
    }
    if (back) {
        story_menu_state.selected_row = StoryMenuRowBack;
        return StoryMenuActionResult::Back;
    }
    if (!confirm) {
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

StoryMenuActionResult apply_story_menu_action_frame(
    StoryMenuState& story_menu_state,
    const bool has_save,
    const ActionInputFrame& input) {
    return apply_story_menu_action(
        story_menu_state,
        has_save,
        input_pressed(input, InputAction::MenuUp),
        input_pressed(input, InputAction::MenuDown),
        input_pressed(input, InputAction::MenuLeft),
        input_pressed(input, InputAction::MenuRight),
        input_pressed(input, InputAction::Confirm),
        input_pressed(input, InputAction::Back));
}

}  // namespace whacker::app
