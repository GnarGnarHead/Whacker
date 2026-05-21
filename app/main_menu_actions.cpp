#include "main_menu_actions.hpp"

namespace whacker::app {

MainMenuActionResult apply_main_menu_action(
    MainMenuState& menu_state,
    const bool move_up,
    const bool move_down,
    const bool confirm,
    const bool back) {
    if (move_up) {
        menu_state.selected_row = (menu_state.selected_row + MainMenuRowCount - 1) % MainMenuRowCount;
    }
    if (move_down) {
        menu_state.selected_row = (menu_state.selected_row + 1) % MainMenuRowCount;
    }
    if (back) {
        return MainMenuActionResult::Quit;
    }
    if (!confirm) {
        return MainMenuActionResult::None;
    }

    switch (menu_state.selected_row) {
        case MainMenuRowStory:
            return MainMenuActionResult::Story;
        case MainMenuRowQuick:
            return MainMenuActionResult::Quick;
        case MainMenuRowOptions:
            return MainMenuActionResult::Options;
        case MainMenuRowQuit:
            return MainMenuActionResult::Quit;
        default:
            return MainMenuActionResult::None;
    }
}

MainMenuActionResult apply_main_menu_action_frame(
    MainMenuState& menu_state,
    const ActionInputFrame& input) {
    return apply_main_menu_action(
        menu_state,
        input_pressed(input, InputAction::MenuUp),
        input_pressed(input, InputAction::MenuDown),
        input_pressed(input, InputAction::Confirm),
        input_pressed(input, InputAction::Back));
}

}  // namespace whacker::app
