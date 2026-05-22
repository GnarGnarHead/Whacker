#include "action_input.hpp"

#include <algorithm>
#include <cmath>
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

bool source_is_controller(const PhysicalInputSource& source) {
    return
        source.kind == PhysicalInputKind::ControllerButton ||
        source.kind == PhysicalInputKind::ControllerAxis ||
        source.kind == PhysicalInputKind::ControllerAxisDirection;
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

float axis_direction_scale(const AxisDirection direction) {
    return direction == AxisDirection::Negative ? -1.0f : 1.0f;
}

float physical_source_value(
    const InputPhysicalState& state,
    const ActionInputBinding& binding) {
    const PhysicalInputSource& source = binding.source;
    switch (source.kind) {
        case PhysicalInputKind::KeyboardScancode:
            return keyboard_scancode_down(state.keyboard, source.keyboard_scancode) ? 1.0f : 0.0f;
        case PhysicalInputKind::ControllerButton:
            return controller_button_down(state, source.controller_index, source.controller_button) ? 1.0f : 0.0f;
        case PhysicalInputKind::ControllerAxis:
            return controller_axis_value(
                state,
                source.controller_index,
                source.controller_axis,
                false,
                binding.deadzone);
        case PhysicalInputKind::ControllerAxisDirection: {
            const float value = controller_axis_value(
                state,
                source.controller_index,
                source.controller_axis,
                false,
                binding.deadzone);
            return source.axis_direction == AxisDirection::Negative ? (value < 0.0f ? 1.0f : 0.0f)
                                                                   : (value > 0.0f ? 1.0f : 0.0f);
        }
    }
    return 0.0f;
}

float binding_output(
    const InputPhysicalState& state,
    const ActionInputBinding& binding) {
    return physical_source_value(state, binding) * binding.output_scale;
}

bool action_down(
    const InputPhysicalState& state,
    const ActionInputBindings& bindings,
    const InputAction action) {
    for (const ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind != InputBindingTargetKind::Action || binding.target.action != action) {
            continue;
        }
        if (std::abs(binding_output(state, binding)) > 0.0f) {
            return true;
        }
    }
    return false;
}

float move_axis(
    const InputPhysicalState& state,
    const ActionInputBindings& bindings,
    const InputSlot slot) {
    float axis = 0.0f;
    for (const ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind != InputBindingTargetKind::MoveY || binding.target.slot != slot) {
            continue;
        }
        axis += binding_output(state, binding);
    }
    return std::clamp(axis, -1.0f, 1.0f);
}

int default_controller_index_for_slot(const InputSlot slot) {
    return slot == InputSlot::P1 ? 0 : 1;
}

AxisDirection output_direction(const float output_scale) {
    return output_scale < 0.0f ? AxisDirection::Negative : AxisDirection::Positive;
}

ActionInputBinding* find_controller_button_move_binding(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const AxisDirection direction) {
    for (ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::MoveY &&
            binding.target.slot == slot &&
            binding.source.kind == PhysicalInputKind::ControllerButton &&
            output_direction(binding.output_scale) == direction) {
            return &binding;
        }
    }
    return nullptr;
}

const ActionInputBinding* find_controller_button_move_binding(
    const ActionInputBindings& bindings,
    const InputSlot slot,
    const AxisDirection direction) {
    for (const ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::MoveY &&
            binding.target.slot == slot &&
            binding.source.kind == PhysicalInputKind::ControllerButton &&
            output_direction(binding.output_scale) == direction) {
            return &binding;
        }
    }
    return nullptr;
}

ActionInputBinding* find_keyboard_move_binding(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const AxisDirection direction) {
    for (ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::MoveY &&
            binding.target.slot == slot &&
            binding.source.kind == PhysicalInputKind::KeyboardScancode &&
            output_direction(binding.output_scale) == direction) {
            return &binding;
        }
    }
    return nullptr;
}

const ActionInputBinding* find_keyboard_move_binding(
    const ActionInputBindings& bindings,
    const InputSlot slot,
    const AxisDirection direction) {
    for (const ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::MoveY &&
            binding.target.slot == slot &&
            binding.source.kind == PhysicalInputKind::KeyboardScancode &&
            output_direction(binding.output_scale) == direction) {
            return &binding;
        }
    }
    return nullptr;
}

ActionInputBinding* find_controller_axis_move_binding(ActionInputBindings& bindings, const InputSlot slot) {
    for (ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::MoveY &&
            binding.target.slot == slot &&
            binding.source.kind == PhysicalInputKind::ControllerAxis) {
            return &binding;
        }
    }
    return nullptr;
}

const ActionInputBinding* find_controller_axis_move_binding(
    const ActionInputBindings& bindings,
    const InputSlot slot) {
    for (const ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::MoveY &&
            binding.target.slot == slot &&
            binding.source.kind == PhysicalInputKind::ControllerAxis) {
            return &binding;
        }
    }
    return nullptr;
}

void set_first_controller_button_for_action(
    ActionInputBindings& bindings,
    const InputAction action,
    const ControllerButton button) {
    for (ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::Action &&
            binding.target.action == action &&
            binding.source.kind == PhysicalInputKind::ControllerButton) {
            binding.source.controller_button = button;
            return;
        }
    }
    add_action_input_binding(
        bindings,
        action_binding_target(action),
        controller_button_source(0, button));
}

void set_controller_axis_direction_for_action(
    ActionInputBindings& bindings,
    const InputAction action,
    const ControllerAxis axis,
    const AxisDirection direction) {
    for (ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::Action &&
            binding.target.action == action &&
            binding.source.kind == PhysicalInputKind::ControllerAxisDirection &&
            binding.source.axis_direction == direction) {
            binding.source.controller_axis = axis;
            return;
        }
    }
    add_action_input_binding(
        bindings,
        action_binding_target(action),
        controller_axis_direction_source(0, axis, direction));
}

}  // namespace

InputBindingTarget action_binding_target(const InputAction action) {
    return InputBindingTarget {
        .kind = InputBindingTargetKind::Action,
        .action = action,
        .slot = InputSlot::P1,
    };
}

InputBindingTarget move_y_binding_target(const InputSlot slot) {
    return InputBindingTarget {
        .kind = InputBindingTargetKind::MoveY,
        .action = InputAction::MenuUp,
        .slot = slot,
    };
}

PhysicalInputSource keyboard_scancode_source(const int scancode) {
    return PhysicalInputSource {
        .kind = PhysicalInputKind::KeyboardScancode,
        .keyboard_scancode = scancode,
    };
}

PhysicalInputSource controller_button_source(
    const int controller_index,
    const ControllerButton button) {
    return PhysicalInputSource {
        .kind = PhysicalInputKind::ControllerButton,
        .controller_index = controller_index,
        .controller_button = button,
    };
}

PhysicalInputSource controller_axis_source(
    const int controller_index,
    const ControllerAxis axis) {
    return PhysicalInputSource {
        .kind = PhysicalInputKind::ControllerAxis,
        .controller_index = controller_index,
        .controller_axis = axis,
    };
}

PhysicalInputSource controller_axis_direction_source(
    const int controller_index,
    const ControllerAxis axis,
    const AxisDirection direction) {
    return PhysicalInputSource {
        .kind = PhysicalInputKind::ControllerAxisDirection,
        .controller_index = controller_index,
        .controller_axis = axis,
        .axis_direction = direction,
    };
}

void add_action_input_binding(
    ActionInputBindings& bindings,
    const InputBindingTarget target,
    const PhysicalInputSource source,
    const float output_scale,
    const float deadzone) {
    bindings.bindings.push_back(ActionInputBinding {
        .target = target,
        .source = source,
        .output_scale = output_scale,
        .deadzone = deadzone,
    });
}

ActionInputBindings default_action_input_bindings() {
    // Controller indices are data, not policy: desktop defaults use 0/P1 and 1/P2,
    // while handheld builds can bind controller 0 to the story/P1 side.
    ActionInputBindings bindings {};

    add_action_input_binding(bindings, action_binding_target(InputAction::MenuUp), keyboard_scancode_source(kKeyboardScancodeUp));
    add_action_input_binding(bindings, action_binding_target(InputAction::MenuUp), keyboard_scancode_source(kKeyboardScancodeW));
    add_action_input_binding(bindings, action_binding_target(InputAction::MenuDown), keyboard_scancode_source(kKeyboardScancodeDown));
    add_action_input_binding(bindings, action_binding_target(InputAction::MenuDown), keyboard_scancode_source(kKeyboardScancodeS));
    add_action_input_binding(bindings, action_binding_target(InputAction::MenuLeft), keyboard_scancode_source(kKeyboardScancodeLeft));
    add_action_input_binding(bindings, action_binding_target(InputAction::MenuRight), keyboard_scancode_source(kKeyboardScancodeRight));
    add_action_input_binding(bindings, action_binding_target(InputAction::Confirm), keyboard_scancode_source(kKeyboardScancodeReturn));
    add_action_input_binding(bindings, action_binding_target(InputAction::Confirm), keyboard_scancode_source(kKeyboardScancodeSpace));
    add_action_input_binding(bindings, action_binding_target(InputAction::Confirm), keyboard_scancode_source(kKeyboardScancodeKpEnter));
    add_action_input_binding(bindings, action_binding_target(InputAction::Back), keyboard_scancode_source(kKeyboardScancodeEscape));
    add_action_input_binding(bindings, action_binding_target(InputAction::Pause), keyboard_scancode_source(kKeyboardScancodeEscape));

    add_action_input_binding(bindings, action_binding_target(InputAction::MenuUp), controller_button_source(0, ControllerButton::DpadUp));
    add_action_input_binding(
        bindings,
        action_binding_target(InputAction::MenuUp),
        controller_axis_direction_source(0, ControllerAxis::LeftY, AxisDirection::Negative));
    add_action_input_binding(bindings, action_binding_target(InputAction::MenuDown), controller_button_source(0, ControllerButton::DpadDown));
    add_action_input_binding(
        bindings,
        action_binding_target(InputAction::MenuDown),
        controller_axis_direction_source(0, ControllerAxis::LeftY, AxisDirection::Positive));
    add_action_input_binding(bindings, action_binding_target(InputAction::MenuLeft), controller_button_source(0, ControllerButton::DpadLeft));
    add_action_input_binding(
        bindings,
        action_binding_target(InputAction::MenuLeft),
        controller_axis_direction_source(0, ControllerAxis::LeftX, AxisDirection::Negative));
    add_action_input_binding(bindings, action_binding_target(InputAction::MenuRight), controller_button_source(0, ControllerButton::DpadRight));
    add_action_input_binding(
        bindings,
        action_binding_target(InputAction::MenuRight),
        controller_axis_direction_source(0, ControllerAxis::LeftX, AxisDirection::Positive));
    add_action_input_binding(bindings, action_binding_target(InputAction::Confirm), controller_button_source(0, ControllerButton::A));
    add_action_input_binding(bindings, action_binding_target(InputAction::Back), controller_button_source(0, ControllerButton::B));
    add_action_input_binding(bindings, action_binding_target(InputAction::Back), controller_button_source(0, ControllerButton::Back));
    add_action_input_binding(bindings, action_binding_target(InputAction::Pause), controller_button_source(0, ControllerButton::Start));

    add_action_input_binding(
        bindings,
        move_y_binding_target(InputSlot::P1),
        keyboard_scancode_source(kKeyboardScancodeW),
        -1.0f);
    add_action_input_binding(
        bindings,
        move_y_binding_target(InputSlot::P1),
        keyboard_scancode_source(kKeyboardScancodeS),
        1.0f);
    add_action_input_binding(
        bindings,
        move_y_binding_target(InputSlot::P2),
        keyboard_scancode_source(kKeyboardScancodeUp),
        -1.0f);
    add_action_input_binding(
        bindings,
        move_y_binding_target(InputSlot::P2),
        keyboard_scancode_source(kKeyboardScancodeDown),
        1.0f);

    add_action_input_binding(bindings, move_y_binding_target(InputSlot::P1), controller_axis_source(0, ControllerAxis::LeftY));
    add_action_input_binding(bindings, move_y_binding_target(InputSlot::P1), controller_button_source(0, ControllerButton::DpadUp), -1.0f);
    add_action_input_binding(bindings, move_y_binding_target(InputSlot::P1), controller_button_source(0, ControllerButton::DpadDown), 1.0f);
    add_action_input_binding(bindings, move_y_binding_target(InputSlot::P2), controller_axis_source(1, ControllerAxis::LeftY));
    add_action_input_binding(bindings, move_y_binding_target(InputSlot::P2), controller_button_source(1, ControllerButton::DpadUp), -1.0f);
    add_action_input_binding(bindings, move_y_binding_target(InputSlot::P2), controller_button_source(1, ControllerButton::DpadDown), 1.0f);

    return bindings;
}

void bind_menu_controller(ActionInputBindings& bindings, const int controller_index) {
    for (ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::Action && source_is_controller(binding.source)) {
            binding.source.controller_index = controller_index;
        }
    }
}

void bind_menu_axes(
    ActionInputBindings& bindings,
    const ControllerAxis x_axis,
    const ControllerAxis y_axis) {
    set_controller_axis_direction_for_action(bindings, InputAction::MenuUp, y_axis, AxisDirection::Negative);
    set_controller_axis_direction_for_action(bindings, InputAction::MenuDown, y_axis, AxisDirection::Positive);
    set_controller_axis_direction_for_action(bindings, InputAction::MenuLeft, x_axis, AxisDirection::Negative);
    set_controller_axis_direction_for_action(bindings, InputAction::MenuRight, x_axis, AxisDirection::Positive);
}

void bind_menu_direction_buttons(
    ActionInputBindings& bindings,
    const ControllerButton up_button,
    const ControllerButton down_button,
    const ControllerButton left_button,
    const ControllerButton right_button) {
    set_first_controller_button_for_action(bindings, InputAction::MenuUp, up_button);
    set_first_controller_button_for_action(bindings, InputAction::MenuDown, down_button);
    set_first_controller_button_for_action(bindings, InputAction::MenuLeft, left_button);
    set_first_controller_button_for_action(bindings, InputAction::MenuRight, right_button);
}

void bind_player_controller(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const int controller_index) {
    bind_controller_index_for_input_slot(bindings, slot, controller_index);
}

void bind_player_move_axis(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const ControllerAxis axis,
    const bool invert_axis) {
    ActionInputBinding* binding = find_controller_axis_move_binding(bindings, slot);
    if (binding == nullptr) {
        add_action_input_binding(
            bindings,
            move_y_binding_target(slot),
            controller_axis_source(controller_index_for_input_slot(bindings, slot), axis),
            invert_axis ? -1.0f : 1.0f);
        return;
    }
    binding->source.controller_axis = axis;
    binding->output_scale = invert_axis ? -1.0f : 1.0f;
}

void bind_player_move_buttons(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const ControllerButton up_button,
    const ControllerButton down_button) {
    (void)bind_controller_button_for_move_direction(bindings, slot, AxisDirection::Negative, up_button);
    (void)bind_controller_button_for_move_direction(bindings, slot, AxisDirection::Positive, down_button);
}

bool bind_keyboard_scancode_for_move_direction(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const AxisDirection direction,
    const int scancode) {
    if (!keyboard_scancode_bindable(scancode)) {
        return false;
    }
    ActionInputBinding* binding = find_keyboard_move_binding(bindings, slot, direction);
    if (binding == nullptr) {
        add_action_input_binding(
            bindings,
            move_y_binding_target(slot),
            keyboard_scancode_source(scancode),
            axis_direction_scale(direction));
        return true;
    }
    binding->source.keyboard_scancode = scancode;
    return true;
}

bool bind_controller_button_for_move_direction(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const AxisDirection direction,
    const ControllerButton button) {
    ActionInputBinding* binding = find_controller_button_move_binding(bindings, slot, direction);
    if (binding == nullptr) {
        add_action_input_binding(
            bindings,
            move_y_binding_target(slot),
            controller_button_source(controller_index_for_input_slot(bindings, slot), button),
            axis_direction_scale(direction));
        return true;
    }
    binding->source.controller_button = button;
    return true;
}

void bind_controller_index_for_input_slot(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const int controller_index) {
    bool updated = false;
    for (ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::MoveY &&
            binding.target.slot == slot &&
            source_is_controller(binding.source)) {
            binding.source.controller_index = controller_index;
            updated = true;
        }
    }
    if (!updated) {
        add_action_input_binding(
            bindings,
            move_y_binding_target(slot),
            controller_axis_source(controller_index, ControllerAxis::LeftY));
    }
}

int keyboard_scancode_for_move_direction(
    const ActionInputBindings& bindings,
    const InputSlot slot,
    const AxisDirection direction) {
    const ActionInputBinding* binding = find_keyboard_move_binding(bindings, slot, direction);
    return binding != nullptr ? binding->source.keyboard_scancode : kKeyboardScancodeUnbound;
}

ControllerButton controller_button_for_move_direction(
    const ActionInputBindings& bindings,
    const InputSlot slot,
    const AxisDirection direction) {
    const ActionInputBinding* binding = find_controller_button_move_binding(bindings, slot, direction);
    return binding != nullptr ? binding->source.controller_button : ControllerButton::Unbound;
}

int controller_index_for_input_slot(const ActionInputBindings& bindings, const InputSlot slot) {
    for (const ActionInputBinding& binding : bindings.bindings) {
        if (binding.target.kind == InputBindingTargetKind::MoveY &&
            binding.target.slot == slot &&
            source_is_controller(binding.source)) {
            return binding.source.controller_index;
        }
    }
    return default_controller_index_for_slot(slot);
}

ControllerAxis controller_axis_for_input_slot(const ActionInputBindings& bindings, const InputSlot slot) {
    const ActionInputBinding* binding = find_controller_axis_move_binding(bindings, slot);
    return binding != nullptr ? binding->source.controller_axis : ControllerAxis::LeftY;
}

bool controller_axis_inverted_for_input_slot(const ActionInputBindings& bindings, const InputSlot slot) {
    const ActionInputBinding* binding = find_controller_axis_move_binding(bindings, slot);
    return binding != nullptr && binding->output_scale < 0.0f;
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
    return state.scancodes[static_cast<std::size_t>(scancode)];
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

    frame.p1_move_y = move_axis(current, bindings, InputSlot::P1);
    frame.p2_move_y = move_axis(current, bindings, InputSlot::P2);
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
