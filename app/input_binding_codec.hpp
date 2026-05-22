#pragma once

#include <iosfwd>
#include <string_view>

#include "action_input.hpp"
#include "menu_input.hpp"

namespace whacker::app {

struct InputBindingLoadState {
    bool loaded_p1_up_scancode = false;
    bool loaded_p1_down_scancode = false;
    bool loaded_p2_up_scancode = false;
    bool loaded_p2_down_scancode = false;
};

bool parse_keyboard_scancode_binding(std::string_view value, int& binding);
bool parse_controller_button_binding(std::string_view value, ControllerButton& button);
int keyboard_scancode_from_legacy_key(int key);
int legacy_key_from_keyboard_scancode(int scancode);
int clamped_controller_index(int index);

bool apply_input_binding_setting(
    ActionInputBindings& bindings,
    ControlBindings& controls,
    InputBindingLoadState& load_state,
    std::string_view key,
    std::string_view value);

bool apply_input_binding_setting(
    ActionInputBindings& bindings,
    ControlBindings& controls,
    std::string_view key,
    std::string_view value);

void write_input_binding_settings(
    std::ostream& output,
    const ControlBindings& controls,
    const ActionInputBindings& bindings);

void sync_controls_from_action_bindings(
    ControlBindings& controls,
    const ActionInputBindings& bindings);

}  // namespace whacker::app
