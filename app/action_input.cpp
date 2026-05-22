#include "action_input.hpp"

#include <algorithm>
#include <cstddef>

namespace whacker::app {

namespace {

int action_index(const InputAction action) {
    return static_cast<int>(action);
}

int controller_axis_index(const ControllerAxis axis) {
    return static_cast<int>(axis);
}

int controller_button_index(const ControllerButton button) {
    return static_cast<int>(button);
}

bool valid_controller_index(const int controller_index) {
    return controller_index >= 0 && controller_index < kMaxInputControllers;
}

bool valid_button(const ControllerButton button) {
    return button != ControllerButton::Unbound && controller_button_index(button) >= 0 &&
        controller_button_index(button) < kControllerButtonCount;
}

bool valid_axis(const ControllerAxis axis) {
    return controller_axis_index(axis) >= 0 && controller_axis_index(axis) < kControllerAxisCount;
}

bool legacy_keyboard_flag_down(const KeyboardPhysicalState& state, const int scancode) {
    switch (scancode) {
        case kKeyboardScancodeW:
            return state.key_w;
        case kKeyboardScancodeS:
            return state.key_s;
        case kKeyboardScancodeUp:
            return state.key_up;
        case kKeyboardScancodeDown:
            return state.key_down;
        case kKeyboardScancodeLeft:
            return state.key_left;
        case kKeyboardScancodeRight:
            return state.key_right;
        case kKeyboardScancodeReturn:
            return state.key_enter;
        case kKeyboardScancodeKpEnter:
            return state.key_kp_enter;
        case kKeyboardScancodeSpace:
            return state.key_space;
        case kKeyboardScancodeEscape:
            return state.key_escape;
        default:
            return false;
    }
}

bool action_down(const KeyboardPhysicalState& state, const ActionInputBindings& bindings, const InputAction action) {
    switch (action) {
        case InputAction::MenuUp:
            return
                keyboard_scancode_down(state, bindings.menu_up_key) ||
                keyboard_scancode_down(state, bindings.menu_secondary_up_key);
        case InputAction::MenuDown:
            return
                keyboard_scancode_down(state, bindings.menu_down_key) ||
                keyboard_scancode_down(state, bindings.menu_secondary_down_key);
        case InputAction::MenuLeft:
            return keyboard_scancode_down(state, bindings.menu_left_key);
        case InputAction::MenuRight:
            return keyboard_scancode_down(state, bindings.menu_right_key);
        case InputAction::Confirm:
            return
                keyboard_scancode_down(state, bindings.menu_confirm_key) ||
                keyboard_scancode_down(state, bindings.menu_secondary_confirm_key) ||
                keyboard_scancode_down(state, bindings.menu_tertiary_confirm_key);
        case InputAction::Back:
            return keyboard_scancode_down(state, bindings.menu_back_key);
        case InputAction::Pause:
            return keyboard_scancode_down(state, bindings.pause_key);
        case InputAction::Count:
            return false;
    }
    return false;
}

float keyboard_axis(const bool negative, const bool positive) {
    const int value = static_cast<int>(positive) - static_cast<int>(negative);
    return std::clamp(static_cast<float>(value), -1.0f, 1.0f);
}

const ControllerPhysicalState* controller_state(
    const InputPhysicalState& state,
    const int controller_index) {
    if (!valid_controller_index(controller_index)) {
        return nullptr;
    }
    const ControllerPhysicalState& controller = state.controllers[static_cast<std::size_t>(controller_index)];
    return controller.connected ? &controller : nullptr;
}

bool controller_button_down(
    const InputPhysicalState& state,
    const int controller_index,
    const ControllerButton button) {
    const ControllerPhysicalState* controller = controller_state(state, controller_index);
    if (controller == nullptr || !valid_button(button)) {
        return false;
    }
    return controller->buttons[static_cast<std::size_t>(controller_button_index(button))];
}

float controller_axis_value(
    const InputPhysicalState& state,
    const int controller_index,
    const ControllerAxis axis,
    const bool invert_axis,
    const float deadzone) {
    const ControllerPhysicalState* controller = controller_state(state, controller_index);
    if (controller == nullptr || !valid_axis(axis)) {
        return 0.0f;
    }
    float value = controller->axes[static_cast<std::size_t>(controller_axis_index(axis))];
    if (invert_axis) {
        value = -value;
    }
    if (value > -deadzone && value < deadzone) {
        return 0.0f;
    }
    return std::clamp(value, -1.0f, 1.0f);
}

float controller_button_axis(
    const InputPhysicalState& state,
    const int controller_index,
    const ControllerButton negative_button,
    const ControllerButton positive_button) {
    return keyboard_axis(
        controller_button_down(state, controller_index, negative_button),
        controller_button_down(state, controller_index, positive_button));
}

bool controller_axis_negative(
    const InputPhysicalState& state,
    const int controller_index,
    const ControllerAxis axis,
    const float deadzone) {
    return controller_axis_value(state, controller_index, axis, false, deadzone) < 0.0f;
}

bool controller_axis_positive(
    const InputPhysicalState& state,
    const int controller_index,
    const ControllerAxis axis,
    const float deadzone) {
    return controller_axis_value(state, controller_index, axis, false, deadzone) > 0.0f;
}

bool action_down(
    const InputPhysicalState& state,
    const ActionInputBindings& bindings,
    const InputAction action) {
    const KeyboardPhysicalState& keyboard = state.keyboard;
    const int menu_controller = bindings.menu_controller_index;
    switch (action) {
        case InputAction::MenuUp:
            return action_down(keyboard, bindings, action) ||
                controller_button_down(state, menu_controller, bindings.menu_up_button) ||
                controller_axis_negative(state, menu_controller, bindings.menu_y_axis, bindings.controller_axis_deadzone);
        case InputAction::MenuDown:
            return action_down(keyboard, bindings, action) ||
                controller_button_down(state, menu_controller, bindings.menu_down_button) ||
                controller_axis_positive(state, menu_controller, bindings.menu_y_axis, bindings.controller_axis_deadzone);
        case InputAction::MenuLeft:
            return action_down(keyboard, bindings, action) ||
                controller_button_down(state, menu_controller, bindings.menu_left_button) ||
                controller_axis_negative(state, menu_controller, bindings.menu_x_axis, bindings.controller_axis_deadzone);
        case InputAction::MenuRight:
            return action_down(keyboard, bindings, action) ||
                controller_button_down(state, menu_controller, bindings.menu_right_button) ||
                controller_axis_positive(state, menu_controller, bindings.menu_x_axis, bindings.controller_axis_deadzone);
        case InputAction::Confirm:
            return action_down(keyboard, bindings, action) ||
                controller_button_down(state, menu_controller, bindings.menu_confirm_button);
        case InputAction::Back:
            return action_down(keyboard, bindings, action) ||
                controller_button_down(state, menu_controller, bindings.menu_back_button) ||
                controller_button_down(state, menu_controller, bindings.menu_secondary_back_button);
        case InputAction::Pause:
            return action_down(keyboard, bindings, action) ||
                controller_button_down(state, menu_controller, bindings.pause_button);
        case InputAction::Count:
            return false;
    }
    return false;
}

float player_axis(
    const InputPhysicalState& state,
    const ControllerPlayerBinding& binding,
    const float deadzone) {
    const float analog = controller_axis_value(
        state,
        binding.controller_index,
        binding.move_y_axis,
        binding.invert_move_y_axis,
        deadzone);
    const float buttons = controller_button_axis(
        state,
        binding.controller_index,
        binding.move_up_button,
        binding.move_down_button);
    return std::clamp(analog + buttons, -1.0f, 1.0f);
}

ControllerPlayerBinding& player_binding(ActionInputBindings& bindings, const PlayerSlot player) {
    return player == PlayerSlot::P1 ? bindings.p1_controller : bindings.p2_controller;
}

}  // namespace

ActionInputBindings default_action_input_bindings() {
    // Controller indices are data, not policy: desktop defaults use 0/P1 and 1/P2,
    // while handheld builds can bind controller 0 to the story/P1 side.
    return ActionInputBindings {};
}

void bind_menu_controller(ActionInputBindings& bindings, const int controller_index) {
    bindings.menu_controller_index = controller_index;
}

void bind_menu_axes(
    ActionInputBindings& bindings,
    const ControllerAxis x_axis,
    const ControllerAxis y_axis) {
    bindings.menu_x_axis = x_axis;
    bindings.menu_y_axis = y_axis;
}

void bind_menu_direction_buttons(
    ActionInputBindings& bindings,
    const ControllerButton up_button,
    const ControllerButton down_button,
    const ControllerButton left_button,
    const ControllerButton right_button) {
    bindings.menu_up_button = up_button;
    bindings.menu_down_button = down_button;
    bindings.menu_left_button = left_button;
    bindings.menu_right_button = right_button;
}

void bind_player_controller(
    ActionInputBindings& bindings,
    const PlayerSlot player,
    const int controller_index) {
    player_binding(bindings, player).controller_index = controller_index;
}

void bind_player_move_axis(
    ActionInputBindings& bindings,
    const PlayerSlot player,
    const ControllerAxis axis,
    const bool invert_axis) {
    ControllerPlayerBinding& binding = player_binding(bindings, player);
    binding.move_y_axis = axis;
    binding.invert_move_y_axis = invert_axis;
}

void bind_player_move_buttons(
    ActionInputBindings& bindings,
    const PlayerSlot player,
    const ControllerButton up_button,
    const ControllerButton down_button) {
    ControllerPlayerBinding& binding = player_binding(bindings, player);
    binding.move_up_button = up_button;
    binding.move_down_button = down_button;
}

bool keyboard_scancode_bindable(const int scancode) {
    if (scancode <= 0 || scancode >= kKeyboardScancodeCount) {
        return false;
    }
    return
        scancode != kKeyboardScancodeReturn &&
        scancode != kKeyboardScancodeKpEnter &&
        scancode != kKeyboardScancodeEscape;
}

bool keyboard_scancode_down(const KeyboardPhysicalState& state, const int scancode) {
    if (scancode < 0 || scancode >= kKeyboardScancodeCount) {
        return false;
    }
    return state.scancodes[static_cast<std::size_t>(scancode)] || legacy_keyboard_flag_down(state, scancode);
}

ActionInputFrame derive_action_input_frame(
    const InputPhysicalState& previous,
    const InputPhysicalState& current) {
    return derive_action_input_frame(previous, current, default_action_input_bindings());
}

ActionInputFrame derive_action_input_frame(
    const InputPhysicalState& previous,
    const InputPhysicalState& current,
    const ActionInputBindings& bindings) {
    ActionInputFrame frame {};
    for (int i = 0; i < kInputActionCount; ++i) {
        const InputAction action = static_cast<InputAction>(i);
        const bool was_down = action_down(previous, bindings, action);
        const bool is_down = action_down(current, bindings, action);
        frame.held[i] = is_down;
        frame.pressed[i] = is_down && !was_down;
        frame.released[i] = !is_down && was_down;
    }

    const float keyboard_p1 = keyboard_axis(
        keyboard_scancode_down(current.keyboard, bindings.p1_move_up_key),
        keyboard_scancode_down(current.keyboard, bindings.p1_move_down_key));
    const float keyboard_p2 = keyboard_axis(
        keyboard_scancode_down(current.keyboard, bindings.p2_move_up_key),
        keyboard_scancode_down(current.keyboard, bindings.p2_move_down_key));
    frame.p1_move_y = std::clamp(
        keyboard_p1 + player_axis(current, bindings.p1_controller, bindings.controller_axis_deadzone),
        -1.0f,
        1.0f);
    frame.p2_move_y = std::clamp(
        keyboard_p2 + player_axis(current, bindings.p2_controller, bindings.controller_axis_deadzone),
        -1.0f,
        1.0f);
    return frame;
}

ActionInputFrame derive_keyboard_action_frame(
    const KeyboardPhysicalState& previous,
    const KeyboardPhysicalState& current) {
    InputPhysicalState previous_state {};
    previous_state.keyboard = previous;
    InputPhysicalState current_state {};
    current_state.keyboard = current;
    return derive_action_input_frame(previous_state, current_state);
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
