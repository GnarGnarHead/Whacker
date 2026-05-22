#pragma once

#include "action_input.hpp"

namespace whacker::app {

struct SdlOptionsBindingRow {
    InputSlot slot = InputSlot::P1;
    AxisDirection direction = AxisDirection::Negative;
};

bool sdl_options_binding_row(int row, SdlOptionsBindingRow& binding_row);
ControllerButton controller_button_for_options_row(const ActionInputBindings& bindings, int row);
int keyboard_scancode_for_options_row(const ActionInputBindings& bindings, int row);
int controller_index_for_options_row(const ActionInputBindings& bindings, int row);
ControllerButton next_bindable_controller_button(ControllerButton current, int direction);

}  // namespace whacker::app
