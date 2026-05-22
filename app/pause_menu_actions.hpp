#pragma once

#include "match_exit_policy.hpp"
#include "menu_intent.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct PauseMenuIntent {
    MenuIntent menu {};
    bool pause = false;
};

enum class PauseMenuActionResult {
    None,
    Resume,
    ExitMatch,
    QuitToMainMenu
};

PauseMenuActionResult apply_pause_menu_action(
    PauseMenuState& pause_menu_state,
    const MatchExitPolicy& exit_policy,
    const PauseMenuIntent& intent);

}  // namespace whacker::app
