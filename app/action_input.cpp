#include "action_input.hpp"

#include <algorithm>

namespace whacker::app {

namespace {

int action_index(const InputAction action) {
    return static_cast<int>(action);
}

bool action_down(const KeyboardPhysicalState& state, const InputAction action) {
    switch (action) {
        case InputAction::MenuUp:
            return state.key_up || state.key_w;
        case InputAction::MenuDown:
            return state.key_down || state.key_s;
        case InputAction::MenuLeft:
            return state.key_left;
        case InputAction::MenuRight:
            return state.key_right;
        case InputAction::Confirm:
            return state.key_enter || state.key_kp_enter || state.key_space;
        case InputAction::Back:
            return state.key_escape;
        case InputAction::Pause:
            return state.key_escape;
        case InputAction::Count:
            return false;
    }
    return false;
}

float keyboard_axis(const bool negative, const bool positive) {
    const int value = static_cast<int>(positive) - static_cast<int>(negative);
    return std::clamp(static_cast<float>(value), -1.0f, 1.0f);
}

}  // namespace

ActionInputFrame derive_keyboard_action_frame(
    const KeyboardPhysicalState& previous,
    const KeyboardPhysicalState& current) {
    ActionInputFrame frame {};
    for (int i = 0; i < kInputActionCount; ++i) {
        const InputAction action = static_cast<InputAction>(i);
        const bool was_down = action_down(previous, action);
        const bool is_down = action_down(current, action);
        frame.held[i] = is_down;
        frame.pressed[i] = is_down && !was_down;
        frame.released[i] = !is_down && was_down;
    }
    frame.p1_move_y = keyboard_axis(current.key_w, current.key_s);
    frame.p2_move_y = keyboard_axis(current.key_up, current.key_down);
    return frame;
}

bool input_held(const ActionInputFrame& frame, const InputAction action) {
    return frame.held[action_index(action)];
}

bool input_pressed(const ActionInputFrame& frame, const InputAction action) {
    return frame.pressed[action_index(action)];
}

bool input_released(const ActionInputFrame& frame, const InputAction action) {
    return frame.released[action_index(action)];
}

}  // namespace whacker::app
