#include "input_binding_codec.hpp"

#include <charconv>
#include <ostream>

namespace whacker::app {

namespace {

bool parse_int_value(const std::string_view text, int& out_value) {
    if (text.empty()) {
        return false;
    }
    int parsed = 0;
    const char* const begin = text.data();
    const char* const end = text.data() + text.size();
    const std::from_chars_result result = std::from_chars(begin, end, parsed, 10);
    if (result.ec != std::errc {} || result.ptr != end) {
        return false;
    }
    out_value = parsed;
    return true;
}

bool apply_keyboard_scancode(
    ActionInputBindings& bindings,
    ControlHintBindings& controls,
    const InputSlot slot,
    const AxisDirection direction,
    bool& loaded_scancode,
    const std::string_view value) {
    int parsed = kKeyboardScancodeUnbound;
    if (!parse_keyboard_scancode_binding(value, parsed)) {
        return false;
    }
    if (!bind_keyboard_scancode_for_move_direction(bindings, slot, direction, parsed)) {
        return false;
    }
    loaded_scancode = true;
    sync_controls_from_action_bindings(controls, bindings);
    return true;
}

bool apply_legacy_key(
    ActionInputBindings& bindings,
    ControlHintBindings& controls,
    const InputSlot slot,
    const AxisDirection direction,
    const bool loaded_scancode,
    const std::string_view value) {
    if (loaded_scancode) {
        return true;
    }
    int parsed = -1;
    if (!parse_int_value(value, parsed)) {
        return false;
    }
    const int scancode = keyboard_scancode_from_legacy_key(parsed);
    if (!keyboard_scancode_bindable(scancode)) {
        return false;
    }
    if (!bind_keyboard_scancode_for_move_direction(bindings, slot, direction, scancode)) {
        return false;
    }
    sync_controls_from_action_bindings(controls, bindings);
    return true;
}

bool apply_controller_button(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const AxisDirection direction,
    const std::string_view value) {
    ControllerButton parsed = ControllerButton::Unbound;
    if (!parse_controller_button_binding(value, parsed)) {
        return false;
    }
    return bind_controller_button_for_move_direction(bindings, slot, direction, parsed);
}

bool parse_boolish_value(const std::string_view value, bool& out_value) {
    if (value == "1" || value == "true" || value == "True" || value == "on") {
        out_value = true;
        return true;
    }
    if (value == "0" || value == "false" || value == "False" || value == "off") {
        out_value = false;
        return true;
    }
    return false;
}

bool apply_controller_axis(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const std::string_view value) {
    ControllerAxis parsed = ControllerAxis::LeftY;
    if (!parse_controller_axis_binding(value, parsed)) {
        return false;
    }
    bind_player_move_axis(bindings, slot, parsed, controller_axis_inverted_for_input_slot(bindings, slot));
    return true;
}

bool apply_controller_axis_invert(
    ActionInputBindings& bindings,
    const InputSlot slot,
    const std::string_view value) {
    bool parsed = false;
    if (!parse_boolish_value(value, parsed)) {
        return false;
    }
    bind_player_move_axis(bindings, slot, controller_axis_for_input_slot(bindings, slot), parsed);
    return true;
}

}  // namespace

bool parse_keyboard_scancode_binding(const std::string_view value, int& binding) {
    int parsed = kKeyboardScancodeUnbound;
    if (!parse_int_value(value, parsed) || !keyboard_scancode_bindable(parsed)) {
        return false;
    }
    binding = parsed;
    return true;
}

bool parse_controller_button_binding(const std::string_view value, ControllerButton& button) {
    int parsed = 0;
    if (!parse_int_value(value, parsed)) {
        return false;
    }
    if (parsed == static_cast<int>(ControllerButton::Unbound)) {
        button = ControllerButton::Unbound;
        return true;
    }
    if (parsed < 0 || parsed >= kControllerButtonCount) {
        return false;
    }
    button = static_cast<ControllerButton>(parsed);
    return true;
}

bool parse_controller_axis_binding(const std::string_view value, ControllerAxis& axis) {
    int parsed = 0;
    if (!parse_int_value(value, parsed)) {
        return false;
    }
    if (parsed < 0 || parsed >= kControllerAxisCount) {
        return false;
    }
    axis = static_cast<ControllerAxis>(parsed);
    return true;
}

int keyboard_scancode_from_legacy_key(const int key) {
    if (key >= 'A' && key <= 'Z') {
        return 4 + (key - 'A');
    }
    switch (key) {
        case 32:
            return kKeyboardScancodeSpace;
        case 262:
            return kKeyboardScancodeRight;
        case 263:
            return kKeyboardScancodeLeft;
        case 264:
            return kKeyboardScancodeDown;
        case 265:
            return kKeyboardScancodeUp;
        default:
            return kKeyboardScancodeUnbound;
    }
}

int legacy_key_from_keyboard_scancode(const int scancode) {
    if (scancode >= 4 && scancode <= 29) {
        return 'A' + (scancode - 4);
    }
    switch (scancode) {
        case kKeyboardScancodeSpace:
            return 32;
        case kKeyboardScancodeRight:
            return 262;
        case kKeyboardScancodeLeft:
            return 263;
        case kKeyboardScancodeDown:
            return 264;
        case kKeyboardScancodeUp:
            return 265;
        default:
            return -1;
    }
}

int clamped_controller_index(const int index) {
    if (index < 0) {
        return 0;
    }
    if (index >= kMaxInputControllers) {
        return kMaxInputControllers - 1;
    }
    return index;
}

bool apply_input_binding_setting(
    ActionInputBindings& bindings,
    ControlHintBindings& controls,
    InputBindingLoadState& load_state,
    const std::string_view key,
    const std::string_view value) {
    if (key == "p1_up_key") {
        return apply_legacy_key(
            bindings,
            controls,
            InputSlot::P1,
            AxisDirection::Negative,
            load_state.loaded_p1_up_scancode,
            value);
    }
    if (key == "p1_down_key") {
        return apply_legacy_key(
            bindings,
            controls,
            InputSlot::P1,
            AxisDirection::Positive,
            load_state.loaded_p1_down_scancode,
            value);
    }
    if (key == "p2_up_key") {
        return apply_legacy_key(
            bindings,
            controls,
            InputSlot::P2,
            AxisDirection::Negative,
            load_state.loaded_p2_up_scancode,
            value);
    }
    if (key == "p2_down_key") {
        return apply_legacy_key(
            bindings,
            controls,
            InputSlot::P2,
            AxisDirection::Positive,
            load_state.loaded_p2_down_scancode,
            value);
    }
    if (key == "p1_up_scancode") {
        return apply_keyboard_scancode(
            bindings,
            controls,
            InputSlot::P1,
            AxisDirection::Negative,
            load_state.loaded_p1_up_scancode,
            value);
    }
    if (key == "p1_down_scancode") {
        return apply_keyboard_scancode(
            bindings,
            controls,
            InputSlot::P1,
            AxisDirection::Positive,
            load_state.loaded_p1_down_scancode,
            value);
    }
    if (key == "p2_up_scancode") {
        return apply_keyboard_scancode(
            bindings,
            controls,
            InputSlot::P2,
            AxisDirection::Negative,
            load_state.loaded_p2_up_scancode,
            value);
    }
    if (key == "p2_down_scancode") {
        return apply_keyboard_scancode(
            bindings,
            controls,
            InputSlot::P2,
            AxisDirection::Positive,
            load_state.loaded_p2_down_scancode,
            value);
    }
    if (key == "p1_controller_index") {
        int parsed = controller_index_for_input_slot(bindings, InputSlot::P1);
        if (!parse_int_value(value, parsed)) {
            return false;
        }
        bind_controller_index_for_input_slot(bindings, InputSlot::P1, clamped_controller_index(parsed));
        return true;
    }
    if (key == "p2_controller_index") {
        int parsed = controller_index_for_input_slot(bindings, InputSlot::P2);
        if (!parse_int_value(value, parsed)) {
            return false;
        }
        bind_controller_index_for_input_slot(bindings, InputSlot::P2, clamped_controller_index(parsed));
        return true;
    }
    if (key == "p1_up_controller_button") {
        return apply_controller_button(bindings, InputSlot::P1, AxisDirection::Negative, value);
    }
    if (key == "p1_down_controller_button") {
        return apply_controller_button(bindings, InputSlot::P1, AxisDirection::Positive, value);
    }
    if (key == "p2_up_controller_button") {
        return apply_controller_button(bindings, InputSlot::P2, AxisDirection::Negative, value);
    }
    if (key == "p2_down_controller_button") {
        return apply_controller_button(bindings, InputSlot::P2, AxisDirection::Positive, value);
    }
    if (key == "p1_controller_axis") {
        return apply_controller_axis(bindings, InputSlot::P1, value);
    }
    if (key == "p2_controller_axis") {
        return apply_controller_axis(bindings, InputSlot::P2, value);
    }
    if (key == "p1_controller_axis_invert") {
        return apply_controller_axis_invert(bindings, InputSlot::P1, value);
    }
    if (key == "p2_controller_axis_invert") {
        return apply_controller_axis_invert(bindings, InputSlot::P2, value);
    }
    return false;
}

bool apply_input_binding_setting(
    ActionInputBindings& bindings,
    ControlHintBindings& controls,
    const std::string_view key,
    const std::string_view value) {
    InputBindingLoadState load_state {};
    return apply_input_binding_setting(bindings, controls, load_state, key, value);
}

void write_input_binding_settings(
    std::ostream& output,
    const ControlHintBindings& controls,
    const ActionInputBindings& bindings) {
    (void)controls;
    output << "p1_up_scancode=" << keyboard_scancode_for_move_direction(
        bindings,
        InputSlot::P1,
        AxisDirection::Negative) << "\n";
    output << "p1_down_scancode=" << keyboard_scancode_for_move_direction(
        bindings,
        InputSlot::P1,
        AxisDirection::Positive) << "\n";
    output << "p2_up_scancode=" << keyboard_scancode_for_move_direction(
        bindings,
        InputSlot::P2,
        AxisDirection::Negative) << "\n";
    output << "p2_down_scancode=" << keyboard_scancode_for_move_direction(
        bindings,
        InputSlot::P2,
        AxisDirection::Positive) << "\n";
    output << "p1_controller_index=" << controller_index_for_input_slot(bindings, InputSlot::P1) << "\n";
    output << "p2_controller_index=" << controller_index_for_input_slot(bindings, InputSlot::P2) << "\n";
    output << "p1_controller_axis=" << static_cast<int>(controller_axis_for_input_slot(bindings, InputSlot::P1)) << "\n";
    output << "p2_controller_axis=" << static_cast<int>(controller_axis_for_input_slot(bindings, InputSlot::P2)) << "\n";
    output << "p1_controller_axis_invert=" << (controller_axis_inverted_for_input_slot(bindings, InputSlot::P1) ? "1" : "0") << "\n";
    output << "p2_controller_axis_invert=" << (controller_axis_inverted_for_input_slot(bindings, InputSlot::P2) ? "1" : "0") << "\n";
    output << "p1_up_controller_button=" << static_cast<int>(controller_button_for_move_direction(
        bindings,
        InputSlot::P1,
        AxisDirection::Negative)) << "\n";
    output << "p1_down_controller_button=" << static_cast<int>(controller_button_for_move_direction(
        bindings,
        InputSlot::P1,
        AxisDirection::Positive)) << "\n";
    output << "p2_up_controller_button=" << static_cast<int>(controller_button_for_move_direction(
        bindings,
        InputSlot::P2,
        AxisDirection::Negative)) << "\n";
    output << "p2_down_controller_button=" << static_cast<int>(controller_button_for_move_direction(
        bindings,
        InputSlot::P2,
        AxisDirection::Positive)) << "\n";
}

void sync_controls_from_action_bindings(
    ControlHintBindings& controls,
    const ActionInputBindings& bindings) {
    controls.p1_up = keyboard_scancode_for_move_direction(
        bindings,
        InputSlot::P1,
        AxisDirection::Negative);
    controls.p1_down = keyboard_scancode_for_move_direction(
        bindings,
        InputSlot::P1,
        AxisDirection::Positive);
    controls.p2_up = keyboard_scancode_for_move_direction(
        bindings,
        InputSlot::P2,
        AxisDirection::Negative);
    controls.p2_down = keyboard_scancode_for_move_direction(
        bindings,
        InputSlot::P2,
        AxisDirection::Positive);
}

}  // namespace whacker::app
