#pragma once

#include "action_input.hpp"
#include "match_exit_policy.hpp"
#include "ui_state.hpp"

namespace whacker::app {

enum class PauseMenuActionResult {
    None,
    Resume,
    ExitMatch,
    QuitToMainMenu
};

PauseMenuActionResult apply_pause_menu_action(
    PauseMenuState& pause_menu_state,
    const MatchExitPolicy& exit_policy,
    bool move_up,
    bool move_down,
    bool move_left,
    bool move_right,
    bool confirm,
    bool back);

PauseMenuActionResult apply_pause_menu_action_frame(
    PauseMenuState& pause_menu_state,
    const MatchExitPolicy& exit_policy,
    const ActionInputFrame& input,
    bool pause_pressed);

}  // namespace whacker::app
