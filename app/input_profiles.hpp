#pragma once

#include "action_input.hpp"

namespace whacker::app {

enum class InputProfile {
    Desktop,
    Handheld
};

const char* input_profile_name(InputProfile profile);
InputProfile configured_input_profile();
ActionInputBindings action_input_bindings_for_profile(InputProfile profile);
ActionInputBindings configured_action_input_bindings();
ActionInputBindings handheld_action_input_bindings();
void apply_handheld_action_input_preset(ActionInputBindings& bindings);

}  // namespace whacker::app
