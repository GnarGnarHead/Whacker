#include "input_profiles.hpp"

#include <algorithm>
#include <initializer_list>

namespace whacker::app {

namespace {

constexpr int kBuiltInControllerIndex = 0;
constexpr int kSecondaryControllerIndex = 1;

bool target_is_action(const ActionInputBinding& binding, const InputAction action) {
    return binding.target.kind == InputBindingTargetKind::Action && binding.target.action == action;
}

bool target_is_move_slot(const ActionInputBinding& binding, const InputSlot slot) {
    return binding.target.kind == InputBindingTargetKind::MoveY && binding.target.slot == slot;
}

bool source_is_controller(const PhysicalInputSource& source) {
    return
        source.kind == PhysicalInputKind::ControllerButton ||
        source.kind == PhysicalInputKind::ControllerAxis ||
        source.kind == PhysicalInputKind::ControllerAxisDirection;
}

void replace_controller_buttons_for_action(
    ActionInputBindings& bindings,
    const InputAction action,
    const std::initializer_list<ControllerButton> buttons) {
    bindings.bindings.erase(
        std::remove_if(
            bindings.bindings.begin(),
            bindings.bindings.end(),
            [action](const ActionInputBinding& binding) {
                return target_is_action(binding, action) &&
                    binding.source.kind == PhysicalInputKind::ControllerButton;
            }),
        bindings.bindings.end());

    for (const ControllerButton button : buttons) {
        if (button == ControllerButton::Unbound) {
            continue;
        }
        add_action_input_binding(
            bindings,
            action_binding_target(action),
            controller_button_source(kBuiltInControllerIndex, button));
    }
}

void replace_controller_axis_direction_for_action(
    ActionInputBindings& bindings,
    const InputAction action,
    const ControllerAxis axis,
    const AxisDirection direction) {
    bindings.bindings.erase(
        std::remove_if(
            bindings.bindings.begin(),
            bindings.bindings.end(),
            [action](const ActionInputBinding& binding) {
                return target_is_action(binding, action) &&
                    binding.source.kind == PhysicalInputKind::ControllerAxisDirection;
            }),
        bindings.bindings.end());

    add_action_input_binding(
        bindings,
        action_binding_target(action),
        controller_axis_direction_source(kBuiltInControllerIndex, axis, direction));
}

void replace_controller_move_bindings_for_slot(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const int controller_index,
    const ControllerAxis axis,
    const ControllerButton up_button,
    const ControllerButton down_button) {
    bindings.bindings.erase(
        std::remove_if(
            bindings.bindings.begin(),
            bindings.bindings.end(),
            [slot](const ActionInputBinding& binding) {
                return target_is_move_slot(binding, slot) && source_is_controller(binding.source);
            }),
        bindings.bindings.end());

    add_action_input_binding(
        bindings,
        move_y_binding_target(slot),
        controller_axis_source(controller_index, axis));
    add_action_input_binding(
        bindings,
        move_y_binding_target(slot),
        controller_button_source(controller_index, up_button),
        -1.0f);
    add_action_input_binding(
        bindings,
        move_y_binding_target(slot),
        controller_button_source(controller_index, down_button),
        1.0f);
}

}  // namespace

const char* input_profile_name(const InputProfile profile) {
    switch (profile) {
        case InputProfile::Desktop:
            return "desktop";
        case InputProfile::Handheld:
            return "handheld";
    }
    return "desktop";
}

InputProfile configured_input_profile() {
#ifdef WHACKER_INPUT_PROFILE_HANDHELD
    return InputProfile::Handheld;
#else
    return InputProfile::Desktop;
#endif
}

ActionInputBindings action_input_bindings_for_profile(const InputProfile profile) {
    switch (profile) {
        case InputProfile::Desktop:
            return default_action_input_bindings();
        case InputProfile::Handheld:
            return handheld_action_input_bindings();
    }
    return default_action_input_bindings();
}

ActionInputBindings configured_action_input_bindings() {
    return action_input_bindings_for_profile(configured_input_profile());
}

ActionInputBindings handheld_action_input_bindings() {
    ActionInputBindings bindings = default_action_input_bindings();
    apply_handheld_action_input_preset(bindings);
    return bindings;
}

void apply_handheld_action_input_preset(ActionInputBindings& bindings) {
    replace_controller_buttons_for_action(bindings, InputAction::MenuUp, {ControllerButton::DpadUp});
    replace_controller_axis_direction_for_action(
        bindings,
        InputAction::MenuUp,
        ControllerAxis::LeftY,
        AxisDirection::Negative);

    replace_controller_buttons_for_action(bindings, InputAction::MenuDown, {ControllerButton::DpadDown});
    replace_controller_axis_direction_for_action(
        bindings,
        InputAction::MenuDown,
        ControllerAxis::LeftY,
        AxisDirection::Positive);

    replace_controller_buttons_for_action(bindings, InputAction::MenuLeft, {ControllerButton::DpadLeft});
    replace_controller_axis_direction_for_action(
        bindings,
        InputAction::MenuLeft,
        ControllerAxis::LeftX,
        AxisDirection::Negative);

    replace_controller_buttons_for_action(bindings, InputAction::MenuRight, {ControllerButton::DpadRight});
    replace_controller_axis_direction_for_action(
        bindings,
        InputAction::MenuRight,
        ControllerAxis::LeftX,
        AxisDirection::Positive);

    replace_controller_buttons_for_action(bindings, InputAction::Confirm, {ControllerButton::A});
    replace_controller_buttons_for_action(bindings, InputAction::Back, {ControllerButton::B});
    replace_controller_buttons_for_action(bindings, InputAction::Pause, {ControllerButton::Start});

    replace_controller_move_bindings_for_slot(
        bindings,
        InputSlot::P1,
        kBuiltInControllerIndex,
        ControllerAxis::LeftY,
        ControllerButton::DpadUp,
        ControllerButton::DpadDown);
    replace_controller_move_bindings_for_slot(
        bindings,
        InputSlot::P2,
        kSecondaryControllerIndex,
        ControllerAxis::LeftY,
        ControllerButton::DpadUp,
        ControllerButton::DpadDown);
}

}  // namespace whacker::app
