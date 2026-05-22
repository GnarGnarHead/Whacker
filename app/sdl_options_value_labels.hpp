#pragma once

#include <string>

#include "action_input.hpp"
#include "audio_engine.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct SdlOptionsValueLabelContext {
    const ActionInputBindings* bindings = nullptr;
    const AudioSettings* audio_settings = nullptr;
};

std::string sdl_options_value_label(const OptionsMenuState& menu_state, int row, const void* raw_context);

}  // namespace whacker::app
