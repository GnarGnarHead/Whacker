#pragma once

#include "audio_engine.hpp"
#include "menu_intent.hpp"
#include "ui_state.hpp"

namespace whacker::app {

enum class OptionsMenuActionResult {
    None,
    Back,
    SectionChanged,
    BindingCaptureStarted,
    BindingChanged,
    ControlPresetChanged,
    AudioChanged
};

int options_menu_row_count(OptionsMenuSection section);
bool options_row_is_back(OptionsMenuSection section, int row);
bool options_row_is_control_preset(OptionsMenuSection section, int row);
bool options_row_is_binding(OptionsMenuSection section, int row);
bool options_row_is_axis_invert(OptionsMenuSection section, int row);
bool options_row_is_volume(OptionsMenuSection section, int row);
bool options_row_is_mute(OptionsMenuSection section, int row);
int options_audio_value(const AudioSettings& settings, int row);
bool options_audio_toggle_value(const AudioSettings& settings, int row);

OptionsMenuActionResult apply_options_menu_action(
    OptionsMenuState& options_menu_state,
    AudioSettings& audio_settings,
    const MenuIntent& intent);

}  // namespace whacker::app
