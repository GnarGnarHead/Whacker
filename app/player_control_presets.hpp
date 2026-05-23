#pragma once

#include <cstdint>

#include "action_input.hpp"

namespace whacker::app {

enum class PlayerControlPreset : std::uint8_t {
    SharedKeyboard,
    SeparateControllers,
    SharedController,
    KeyboardVsController,
    Custom
};

const char* player_control_preset_label(PlayerControlPreset preset);
PlayerControlPreset detect_player_control_preset(const ActionInputBindings& bindings);
PlayerControlPreset next_player_control_preset(PlayerControlPreset current, int direction);
bool apply_player_control_preset(ActionInputBindings& bindings, PlayerControlPreset preset);
bool cycle_player_control_preset(ActionInputBindings& bindings, int direction);

}  // namespace whacker::app
