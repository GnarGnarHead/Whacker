#pragma once

#include "action_input.hpp"
#include "control_types.hpp"

namespace whacker::app {

struct MenuInputIntent {
    MenuIntent pressed {};
    MenuIntent held {};
    bool pause = false;
};

inline MenuIntent derive_menu_intent(const ActionInputFrame& input) {
    return MenuIntent {
        .up = input_pressed(input, InputAction::MenuUp),
        .down = input_pressed(input, InputAction::MenuDown),
        .left = input_pressed(input, InputAction::MenuLeft),
        .right = input_pressed(input, InputAction::MenuRight),
        .confirm = input_pressed(input, InputAction::Confirm),
        .back = input_pressed(input, InputAction::Back),
    };
}

inline MenuIntent derive_held_menu_intent(const ActionInputFrame& input) {
    return MenuIntent {
        .up = input_held(input, InputAction::MenuUp),
        .down = input_held(input, InputAction::MenuDown),
        .left = input_held(input, InputAction::MenuLeft),
        .right = input_held(input, InputAction::MenuRight),
        .confirm = input_held(input, InputAction::Confirm),
        .back = input_held(input, InputAction::Back),
    };
}

inline MenuInputIntent derive_menu_input_intent(const ActionInputFrame& input) {
    return MenuInputIntent {
        .pressed = derive_menu_intent(input),
        .held = derive_held_menu_intent(input),
        .pause = input_pressed(input, InputAction::Pause),
    };
}

}  // namespace whacker::app
