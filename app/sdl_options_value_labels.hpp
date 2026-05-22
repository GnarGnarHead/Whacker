#pragma once

#include <string>

#include "action_input.hpp"
#include "audio_engine.hpp"

namespace whacker::app {

struct SdlOptionsValueLabelContext {
    const ActionInputBindings* bindings = nullptr;
    const AudioSettings* audio_settings = nullptr;
};

std::string sdl_options_value_label(int row, const void* raw_context);

}  // namespace whacker::app
