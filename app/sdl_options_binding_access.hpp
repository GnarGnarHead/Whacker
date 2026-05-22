#pragma once

#include "action_input.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct SdlOptionsBindingRow {
    InputSlot slot = InputSlot::P1;
    AxisDirection direction = AxisDirection::Negative;
};

bool sdl_options_binding_row(OptionsMenuSection section, int row, SdlOptionsBindingRow& binding_row);
bool sdl_options_axis_row(OptionsMenuSection section, int row, InputSlot& slot);
bool sdl_options_axis_invert_row(OptionsMenuSection section, int row, InputSlot& slot);
ControllerButton controller_button_for_options_row(
    const ActionInputBindings& bindings,
    OptionsMenuSection section,
    int row);
int keyboard_scancode_for_options_row(
    const ActionInputBindings& bindings,
    OptionsMenuSection section,
    int row);
int controller_index_for_options_row(
    const ActionInputBindings& bindings,
    OptionsMenuSection section,
    int row);
ControllerAxis controller_axis_for_options_row(
    const ActionInputBindings& bindings,
    OptionsMenuSection section,
    int row);
bool controller_axis_inverted_for_options_row(
    const ActionInputBindings& bindings,
    OptionsMenuSection section,
    int row);
ControllerButton next_bindable_controller_button(ControllerButton current, int direction);
ControllerAxis next_bindable_controller_axis(ControllerAxis current, int direction);

}  // namespace whacker::app
