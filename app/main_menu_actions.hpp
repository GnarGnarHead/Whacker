#pragma once

#include "action_input.hpp"
#include "ui_state.hpp"

namespace whacker::app {

enum class MainMenuActionResult {
    None,
    Story,
    Quick,
    Options,
    Quit
};

MainMenuActionResult apply_main_menu_action(
    MainMenuState& menu_state,
    bool move_up,
    bool move_down,
    bool confirm,
    bool back);

MainMenuActionResult apply_main_menu_action_frame(
    MainMenuState& menu_state,
    const ActionInputFrame& input);

}  // namespace whacker::app
