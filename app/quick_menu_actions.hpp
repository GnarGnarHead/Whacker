#pragma once

#include "app_types.hpp"
#include "menu_intent.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct QuickMenuActionResult {
    bool row_changed = false;
    bool options_changed = false;
    bool start_requested = false;
    bool tune_p1_requested = false;
    bool tune_p2_requested = false;
    bool back_requested = false;
};

QuickMenuActionResult apply_quick_menu_action(
    MenuState& menu_state,
    MatchOptions& options,
    const MenuIntent& intent);

}  // namespace whacker::app
