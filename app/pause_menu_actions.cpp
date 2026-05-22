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
    const PauseMenuIntent& intent) {
    const bool back = intent.pause || intent.menu.back;
    if ((!exit_policy.has_exit_option || !exit_policy.can_exit_now || !exit_policy.requires_confirmation) &&
        pause_menu_state.confirm_forfeit) {
        clear_pause_confirmation(pause_menu_state);
    }

    if (pause_menu_state.confirm_forfeit) {
        if (back) {
            clear_pause_confirmation(pause_menu_state);
            return PauseMenuActionResult::None;
        }
        if (intent.menu.up || intent.menu.down || intent.menu.left || intent.menu.right) {
            pause_menu_state.confirm_selected = 1 - pause_menu_state.confirm_selected;
        }
        if (!intent.menu.confirm) {
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
    if (intent.menu.up) {
        pause_menu_state.selected_row = (pause_menu_state.selected_row + row_count - 1) % row_count;
    }
    if (intent.menu.down) {
        pause_menu_state.selected_row = (pause_menu_state.selected_row + 1) % row_count;
    }
    if (back) {
        pause_menu_state.selected_row = PauseMenuRowResume;
        return PauseMenuActionResult::Resume;
    }
    if (!intent.menu.confirm) {
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

}  // namespace whacker::app
