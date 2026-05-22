#pragma once

#include "menu_intent.hpp"
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
    const MenuIntent& intent);

}  // namespace whacker::app
