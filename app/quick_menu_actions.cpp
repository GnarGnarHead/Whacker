#include "quick_menu_actions.hpp"

#include "ai_style_catalog.hpp"

namespace whacker::app {

namespace {

int mode_index(const PaddleMode mode) {
    return mode == PaddleMode::Human ? 0 : 1;
}

PaddleMode mode_from_index(const int index) {
    return index == 0 ? PaddleMode::Human : PaddleMode::AI;
}

void cycle_paddle_mode(PaddleMode& mode, const int direction) {
    const int step = direction >= 0 ? 1 : -1;
    int index = mode_index(mode);
    index = (index + step + 2) % 2;
    mode = mode_from_index(index);
}

void set_menu_row_option(MatchOptions& options, const MenuState& menu_state, const int direction) {
    if (menu_state.selected_row == MenuRowP1) {
        cycle_paddle_mode(options.left_mode, direction);
    } else if (menu_state.selected_row == MenuRowP2) {
        cycle_paddle_mode(options.right_mode, direction);
    } else if (menu_state.selected_row == MenuRowP1Tuning) {
        const int index = (ai_style_index(options.left_ai_style) + direction + kAiStyleCount) % kAiStyleCount;
        options.left_ai_style = ai_style_from_index(index);
        options.left_paddle_skills = ai_style_profile(options.left_ai_style).seed_skills;
    } else if (menu_state.selected_row == MenuRowP2Tuning) {
        const int index = (ai_style_index(options.right_ai_style) + direction + kAiStyleCount) % kAiStyleCount;
        options.right_ai_style = ai_style_from_index(index);
        options.right_paddle_skills = ai_style_profile(options.right_ai_style).seed_skills;
    }
}

}  // namespace

QuickMenuActionResult apply_quick_menu_action_frame(
    MenuState& menu_state,
    MatchOptions& options,
    const ActionInputFrame& input) {
    if (input_pressed(input, InputAction::MenuUp)) {
        menu_state.selected_row = (menu_state.selected_row + MenuRowCount - 1) % MenuRowCount;
    }
    if (input_pressed(input, InputAction::MenuDown)) {
        menu_state.selected_row = (menu_state.selected_row + 1) % MenuRowCount;
    }

    int direction = 0;
    if (input_pressed(input, InputAction::MenuLeft)) {
        direction -= 1;
    }
    if (input_pressed(input, InputAction::MenuRight)) {
        direction += 1;
    }
    if (direction != 0) {
        set_menu_row_option(options, menu_state, direction);
    }

    if (!input_pressed(input, InputAction::Confirm)) {
        return QuickMenuActionResult::None;
    }
    if (menu_state.selected_row == MenuRowStart) {
        return QuickMenuActionResult::StartMatch;
    }
    if (menu_state.selected_row == MenuRowP1Tuning) {
        return QuickMenuActionResult::TuneP1;
    }
    if (menu_state.selected_row == MenuRowP2Tuning) {
        return QuickMenuActionResult::TuneP2;
    }
    set_menu_row_option(options, menu_state, 1);
    return QuickMenuActionResult::None;
}

}  // namespace whacker::app
