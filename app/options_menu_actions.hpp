#pragma once

#include "action_input.hpp"
#include "audio_engine.hpp"
#include "ui_state.hpp"

namespace whacker::app {

enum class OptionsMenuActionResult {
    None,
    Back,
    BindingCaptureStarted,
    BindingChanged,
    AudioChanged
};

bool options_row_is_binding(int row);
bool options_row_is_volume(int row);
bool options_row_is_mute(int row);
int audio_value(const AudioSettings& settings, int row);
bool audio_toggle_value(const AudioSettings& settings, int row);

OptionsMenuActionResult apply_options_menu_action(
    OptionsMenuState& options_menu_state,
    AudioSettings& audio_settings,
    bool move_up,
    bool move_down,
    bool move_left,
    bool move_right,
    bool confirm,
    bool back);

OptionsMenuActionResult apply_options_menu_action_frame(
    OptionsMenuState& options_menu_state,
    AudioSettings& audio_settings,
    const ActionInputFrame& input);

}  // namespace whacker::app
