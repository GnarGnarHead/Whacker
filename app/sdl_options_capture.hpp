#pragma once

#include "action_input.hpp"
#include "menu_input.hpp"
#include "platform_sdl.hpp"
#include "sdl_input.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct SdlOptionsCaptureResult {
    bool finished = false;
    bool binding_changed = false;
};

SdlOptionsCaptureResult apply_sdl_options_capture(
    OptionsMenuState& options_menu_state,
    ActionInputBindings& bindings,
    ControlHintBindings& controls,
    const SdlInput& input,
    const SdlEventFrame& events);

bool cycle_sdl_options_controller_button(
    ActionInputBindings& bindings,
    OptionsMenuSection section,
    int row,
    int direction);
bool cycle_sdl_options_controller_axis(
    ActionInputBindings& bindings,
    OptionsMenuSection section,
    int row,
    int direction);
bool toggle_sdl_options_controller_axis_invert(
    ActionInputBindings& bindings,
    OptionsMenuSection section,
    int row);

}  // namespace whacker::app
