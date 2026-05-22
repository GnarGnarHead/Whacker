#pragma once

#include "action_input.hpp"

namespace whacker::app {

ControllerButton controller_button_for_options_row(const ActionInputBindings& bindings, int row);
int keyboard_scancode_for_options_row(const ActionInputBindings& bindings, int row);
int controller_index_for_options_row(const ActionInputBindings& bindings, int row);
ControllerButton next_bindable_controller_button(ControllerButton current, int direction);

}  // namespace whacker::app
