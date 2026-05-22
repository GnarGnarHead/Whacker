#pragma once

#include "audio_engine.hpp"
#include "menu_intent.hpp"
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
bool options_row_is_axis_invert(int row);
bool options_row_is_volume(int row);
bool options_row_is_mute(int row);
int audio_value(const AudioSettings& settings, int row);
bool audio_toggle_value(const AudioSettings& settings, int row);

OptionsMenuActionResult apply_options_menu_action(
    OptionsMenuState& options_menu_state,
    AudioSettings& audio_settings,
    const MenuIntent& intent);

}  // namespace whacker::app
