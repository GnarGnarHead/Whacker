#pragma once

#include "app_types.hpp"
#include "action_input.hpp"
#include "audio_engine.hpp"
#include "menu_input.hpp"

namespace whacker::app {

void load_menu_settings(MatchOptions& options, ControlBindings& controls, AudioSettings& audio_settings);
void save_menu_settings(const MatchOptions& options, const ControlBindings& controls, const AudioSettings& audio_settings);
void load_menu_settings(
    MatchOptions& options,
    ControlBindings& controls,
    ActionInputBindings& action_bindings,
    AudioSettings& audio_settings);
void save_menu_settings(
    const MatchOptions& options,
    const ControlBindings& controls,
    const ActionInputBindings& action_bindings,
    const AudioSettings& audio_settings);

}  // namespace whacker::app
