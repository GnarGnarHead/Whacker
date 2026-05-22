#include "pause_menu_actions.hpp"

namespace whacker::app {

namespace {

int visible_pause_row_count(const MatchExitPolicy& exit_policy) {
    return exit_policy.has_exit_option ? PauseMenuRowCount : (PauseMenuRowCount - 1);
}

void clamp_pause_selection(PauseMenuState& pause_menu_state, const MatchExitPolicy& exit_policy) {
    const int row_count = visible_pause_row_count(exit_policy);
    if (pause_menu_state.selected_row < 0) {
        pause_menu_state.selected_row = 0;
    }
    if (pause_menu_state.selected_row >= row_count) {
        pause_menu_state.selected_row = row_count - 1;
    }
}

void clear_pause_confirmation(PauseMenuState& pause_menu_state) {
    pause_menu_state.confirm_forfeit = false;
    pause_menu_state.confirm_selected = 0;
}

}  // namespace

PauseMenuActionResult apply_pause_menu_action(
    PauseMenuState& pause_menu_state,
    const MatchExitPolicy& exit_policy,
    const bool move_up,
    const bool move_down,
    const bool move_left,
    const bool move_right,
    const bool confirm,
    const bool back) {
    if ((!exit_policy.has_exit_option || !exit_policy.can_exit_now || !exit_policy.requires_confirmation) &&
        pause_menu_state.confirm_forfeit) {
        clear_pause_confirmation(pause_menu_state);
    }

    if (pause_menu_state.confirm_forfeit) {
        if (back) {
            clear_pause_confirmation(pause_menu_state);
            return PauseMenuActionResult::None;
        }
        if (move_up || move_down || move_left || move_right) {
            pause_menu_state.confirm_selected = 1 - pause_menu_state.confirm_selected;
        }
        if (!confirm) {
            return PauseMenuActionResult::None;
        }
        if (pause_menu_state.confirm_selected == 1) {
            clear_pause_confirmation(pause_menu_state);
            return PauseMenuActionResult::ExitMatch;
        }
        clear_pause_confirmation(pause_menu_state);
        return PauseMenuActionResult::None;
    }

    clamp_pause_selection(pause_menu_state, exit_policy);
    const int row_count = visible_pause_row_count(exit_policy);
    if (move_up) {
        pause_menu_state.selected_row = (pause_menu_state.selected_row + row_count - 1) % row_count;
    }
    if (move_down) {
        pause_menu_state.selected_row = (pause_menu_state.selected_row + 1) % row_count;
    }
    if (back) {
        pause_menu_state.selected_row = PauseMenuRowResume;
        return PauseMenuActionResult::Resume;
    }
    if (!confirm) {
        return PauseMenuActionResult::None;
    }

    if (pause_menu_state.selected_row == PauseMenuRowResume) {
        return PauseMenuActionResult::Resume;
    }

    const bool selected_exit_row =
        exit_policy.has_exit_option && pause_menu_state.selected_row == PauseMenuRowExitMatch;
    if (selected_exit_row) {
        if (!exit_policy.can_exit_now) {
            pause_menu_state.selected_row = PauseMenuRowResume;
            return PauseMenuActionResult::None;
        }
        if (exit_policy.requires_confirmation) {
            pause_menu_state.confirm_forfeit = true;
            pause_menu_state.confirm_selected = 0;
            return PauseMenuActionResult::None;
        }
        return PauseMenuActionResult::ExitMatch;
    }

    return PauseMenuActionResult::QuitToMainMenu;
}

PauseMenuActionResult apply_pause_menu_action_frame(
    PauseMenuState& pause_menu_state,
    const MatchExitPolicy& exit_policy,
    const ActionInputFrame& input,
    const bool pause_pressed) {
    return apply_pause_menu_action(
        pause_menu_state,
        exit_policy,
        input_pressed(input, InputAction::MenuUp),
        input_pressed(input, InputAction::MenuDown),
        input_pressed(input, InputAction::MenuLeft),
        input_pressed(input, InputAction::MenuRight),
        input_pressed(input, InputAction::Confirm),
        pause_pressed || input_pressed(input, InputAction::Back));
}

}  // namespace whacker::app
