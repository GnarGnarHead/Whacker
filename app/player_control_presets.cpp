#include "player_control_presets.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace whacker::app {

namespace {

constexpr std::array<PlayerControlPreset, 4> kSelectablePresets {{
    PlayerControlPreset::SharedKeyboard,
    PlayerControlPreset::SeparateControllers,
    PlayerControlPreset::SharedController,
    PlayerControlPreset::KeyboardVsController,
}};

bool is_player_move_binding(const ActionInputBinding& binding) {
    return binding.target.kind == InputBindingTargetKind::MoveY &&
        (binding.target.slot == InputSlot::P1 || binding.target.slot == InputSlot::P2);
}

void add_keyboard_pair(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const int up_scancode,
    const int down_scancode) {
    add_action_input_binding(
        bindings,
        move_y_binding_target(slot),
        keyboard_scancode_source(up_scancode),
        -1.0f);
    add_action_input_binding(
        bindings,
        move_y_binding_target(slot),
        keyboard_scancode_source(down_scancode),
        1.0f);
}

void add_controller_pair(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const int controller_index,
    const ControllerAxis axis,
    const ControllerButton up_button,
    const ControllerButton down_button) {
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

ActionInputBindings player_move_bindings_for_preset(const PlayerControlPreset preset) {
    ActionInputBindings bindings {};

    switch (preset) {
        case PlayerControlPreset::SharedKeyboard:
            add_keyboard_pair(bindings, InputSlot::P1, kKeyboardScancodeW, kKeyboardScancodeS);
            add_keyboard_pair(bindings, InputSlot::P2, kKeyboardScancodeUp, kKeyboardScancodeDown);
            break;
        case PlayerControlPreset::SeparateControllers:
            add_keyboard_pair(bindings, InputSlot::P1, kKeyboardScancodeW, kKeyboardScancodeS);
            add_keyboard_pair(bindings, InputSlot::P2, kKeyboardScancodeUp, kKeyboardScancodeDown);
            add_controller_pair(
                bindings,
                InputSlot::P1,
                0,
                ControllerAxis::LeftY,
                ControllerButton::DpadUp,
                ControllerButton::DpadDown);
            add_controller_pair(
                bindings,
                InputSlot::P2,
                1,
                ControllerAxis::LeftY,
                ControllerButton::DpadUp,
                ControllerButton::DpadDown);
            break;
        case PlayerControlPreset::SharedController:
            add_keyboard_pair(bindings, InputSlot::P1, kKeyboardScancodeW, kKeyboardScancodeS);
            add_keyboard_pair(bindings, InputSlot::P2, kKeyboardScancodeUp, kKeyboardScancodeDown);
            add_controller_pair(
                bindings,
                InputSlot::P1,
                0,
                ControllerAxis::LeftY,
                ControllerButton::DpadUp,
                ControllerButton::DpadDown);
            add_controller_pair(
                bindings,
                InputSlot::P2,
                0,
                ControllerAxis::RightY,
                ControllerButton::Y,
                ControllerButton::A);
            break;
        case PlayerControlPreset::KeyboardVsController:
            add_keyboard_pair(bindings, InputSlot::P1, kKeyboardScancodeW, kKeyboardScancodeS);
            add_controller_pair(
                bindings,
                InputSlot::P2,
                0,
                ControllerAxis::LeftY,
                ControllerButton::DpadUp,
                ControllerButton::DpadDown);
            break;
        case PlayerControlPreset::Custom:
            break;
    }

    return bindings;
}

bool source_matches(const PhysicalInputSource& a, const PhysicalInputSource& b) {
    return
        a.kind == b.kind &&
        a.keyboard_scancode == b.keyboard_scancode &&
        a.controller_index == b.controller_index &&
        a.controller_button == b.controller_button &&
        a.controller_axis == b.controller_axis &&
        a.axis_direction == b.axis_direction;
}

bool binding_matches(const ActionInputBinding& a, const ActionInputBinding& b) {
    return
        a.target.kind == b.target.kind &&
        a.target.action == b.target.action &&
        a.target.slot == b.target.slot &&
        source_matches(a.source, b.source) &&
        a.output_scale == b.output_scale &&
        a.deadzone == b.deadzone;
}

std::vector<ActionInputBinding> player_move_bindings(const ActionInputBindings& bindings) {
    std::vector<ActionInputBinding> moves {};
    for (const ActionInputBinding& binding : bindings.bindings) {
        if (is_player_move_binding(binding)) {
            moves.push_back(binding);
        }
    }
    return moves;
}

bool player_move_bindings_match(
    const ActionInputBindings& bindings,
    const PlayerControlPreset preset) {
    const std::vector<ActionInputBinding> actual = player_move_bindings(bindings);
    const std::vector<ActionInputBinding> expected =
        player_move_bindings(player_move_bindings_for_preset(preset));
    if (actual.size() != expected.size()) {
        return false;
    }

    std::vector<bool> matched(expected.size(), false);
    for (const ActionInputBinding& actual_binding : actual) {
        bool found = false;
        for (std::size_t i = 0; i < expected.size(); ++i) {
            if (!matched[i] && binding_matches(actual_binding, expected[i])) {
                matched[i] = true;
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

int preset_index(const PlayerControlPreset preset) {
    for (int i = 0; i < static_cast<int>(kSelectablePresets.size()); ++i) {
        if (kSelectablePresets[static_cast<std::size_t>(i)] == preset) {
            return i;
        }
    }
    return -1;
}

void replace_player_move_bindings(
    ActionInputBindings& bindings,
    const ActionInputBindings& player_moves) {
    std::vector<ActionInputBinding> next {};
    next.reserve(bindings.bindings.size() + player_moves.bindings.size());
    for (const ActionInputBinding& binding : bindings.bindings) {
        if (!is_player_move_binding(binding)) {
            next.push_back(binding);
        }
    }
    for (const ActionInputBinding& binding : player_moves.bindings) {
        next.push_back(binding);
    }
    bindings.bindings = next;
}

}  // namespace

const char* player_control_preset_label(const PlayerControlPreset preset) {
    switch (preset) {
        case PlayerControlPreset::SharedKeyboard:
            return "SHARED KEYS";
        case PlayerControlPreset::SeparateControllers:
            return "TWO PADS";
        case PlayerControlPreset::SharedController:
            return "SHARED PAD";
        case PlayerControlPreset::KeyboardVsController:
            return "KEYS VS PAD";
        case PlayerControlPreset::Custom:
            return "CUSTOM";
    }
    return "CUSTOM";
}

PlayerControlPreset detect_player_control_preset(const ActionInputBindings& bindings) {
    for (const PlayerControlPreset preset : kSelectablePresets) {
        if (player_move_bindings_match(bindings, preset)) {
            return preset;
        }
    }
    return PlayerControlPreset::Custom;
}

PlayerControlPreset next_player_control_preset(
    const PlayerControlPreset current,
    const int direction) {
    if (direction == 0) {
        return current;
    }

    int index = preset_index(current);
    if (index < 0) {
        index = direction > 0 ? -1 : 0;
    }
    index = (index + direction + static_cast<int>(kSelectablePresets.size())) %
        static_cast<int>(kSelectablePresets.size());
    return kSelectablePresets[static_cast<std::size_t>(index)];
}

bool apply_player_control_preset(
    ActionInputBindings& bindings,
    const PlayerControlPreset preset) {
    if (preset == PlayerControlPreset::Custom) {
        return false;
    }
    const ActionInputBindings before = bindings;
    replace_player_move_bindings(bindings, player_move_bindings_for_preset(preset));
    return !player_move_bindings_match(before, preset);
}

bool cycle_player_control_preset(ActionInputBindings& bindings, const int direction) {
    if (direction == 0) {
        return false;
    }
    const PlayerControlPreset current = detect_player_control_preset(bindings);
    return apply_player_control_preset(bindings, next_player_control_preset(current, direction));
}

}  // namespace whacker::app
