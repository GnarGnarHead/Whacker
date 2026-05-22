#pragma once

#include "action_input.hpp"

namespace whacker::app {

ActionInputBindings handheld_action_input_bindings();
void apply_handheld_action_input_preset(ActionInputBindings& bindings);

}  // namespace whacker::app
