#include "main_menu_actions.hpp"

namespace whacker::app {

MainMenuActionResult apply_main_menu_action(
    MainMenuState& menu_state,
    const MenuIntent& intent) {
    if (intent.up) {
        menu_state.selected_row = (menu_state.selected_row + MainMenuRowCount - 1) % MainMenuRowCount;
    }
    if (intent.down) {
        menu_state.selected_row = (menu_state.selected_row + 1) % MainMenuRowCount;
    }
    if (intent.back) {
        return MainMenuActionResult::Quit;
    }
    if (!intent.confirm) {
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

}  // namespace whacker::app
