#pragma once

#include "action_input.hpp"
#include "app_types.hpp"
#include "ui_state.hpp"

namespace whacker::app {

enum class QuickMenuActionResult {
    None,
    StartMatch,
    TuneP1,
    TuneP2
};

QuickMenuActionResult apply_quick_menu_action_frame(
    MenuState& menu_state,
    MatchOptions& options,
    const ActionInputFrame& input);

}  // namespace whacker::app
