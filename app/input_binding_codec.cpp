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
    ControlBindings& controls,
    int& action_binding,
    int& control_binding,
    bool& loaded_scancode,
    const std::string_view value) {
    int parsed = kKeyboardScancodeUnbound;
    if (!parse_keyboard_scancode_binding(value, parsed)) {
        return false;
    }
    action_binding = parsed;
    loaded_scancode = true;
    (void)control_binding;
    sync_controls_from_action_bindings(controls, bindings);
    return true;
}

bool apply_legacy_key(
    ActionInputBindings& bindings,
    ControlBindings& controls,
    int& action_binding,
    int& control_binding,
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
    (void)control_binding;
    action_binding = scancode;
    sync_controls_from_action_bindings(controls, bindings);
    return true;
}

bool apply_controller_button(
    ControllerButton& binding,
    const std::string_view value) {
    ControllerButton parsed = ControllerButton::Unbound;
    if (!parse_controller_button_binding(value, parsed)) {
        return false;
    }
    binding = parsed;
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
    ControlBindings& controls,
    InputBindingLoadState& load_state,
    const std::string_view key,
    const std::string_view value) {
    if (key == "p1_up_key") {
        return apply_legacy_key(
            bindings,
            controls,
            bindings.p1_move_up_key,
            controls.p1_up,
            load_state.loaded_p1_up_scancode,
            value);
    }
    if (key == "p1_down_key") {
        return apply_legacy_key(
            bindings,
            controls,
            bindings.p1_move_down_key,
            controls.p1_down,
            load_state.loaded_p1_down_scancode,
            value);
    }
    if (key == "p2_up_key") {
        return apply_legacy_key(
            bindings,
            controls,
            bindings.p2_move_up_key,
            controls.p2_up,
            load_state.loaded_p2_up_scancode,
            value);
    }
    if (key == "p2_down_key") {
        return apply_legacy_key(
            bindings,
            controls,
            bindings.p2_move_down_key,
            controls.p2_down,
            load_state.loaded_p2_down_scancode,
            value);
    }
    if (key == "p1_up_scancode") {
        return apply_keyboard_scancode(
            bindings,
            controls,
            bindings.p1_move_up_key,
            controls.p1_up,
            load_state.loaded_p1_up_scancode,
            value);
    }
    if (key == "p1_down_scancode") {
        return apply_keyboard_scancode(
            bindings,
            controls,
            bindings.p1_move_down_key,
            controls.p1_down,
            load_state.loaded_p1_down_scancode,
            value);
    }
    if (key == "p2_up_scancode") {
        return apply_keyboard_scancode(
            bindings,
            controls,
            bindings.p2_move_up_key,
            controls.p2_up,
            load_state.loaded_p2_up_scancode,
            value);
    }
    if (key == "p2_down_scancode") {
        return apply_keyboard_scancode(
            bindings,
            controls,
            bindings.p2_move_down_key,
            controls.p2_down,
            load_state.loaded_p2_down_scancode,
            value);
    }
    if (key == "p1_controller_index") {
        int parsed = bindings.p1_controller.controller_index;
        if (!parse_int_value(value, parsed)) {
            return false;
        }
        bindings.p1_controller.controller_index = clamped_controller_index(parsed);
        return true;
    }
    if (key == "p2_controller_index") {
        int parsed = bindings.p2_controller.controller_index;
        if (!parse_int_value(value, parsed)) {
            return false;
        }
        bindings.p2_controller.controller_index = clamped_controller_index(parsed);
        return true;
    }
    if (key == "p1_up_controller_button") {
        return apply_controller_button(bindings.p1_controller.move_up_button, value);
    }
    if (key == "p1_down_controller_button") {
        return apply_controller_button(bindings.p1_controller.move_down_button, value);
    }
    if (key == "p2_up_controller_button") {
        return apply_controller_button(bindings.p2_controller.move_up_button, value);
    }
    if (key == "p2_down_controller_button") {
        return apply_controller_button(bindings.p2_controller.move_down_button, value);
    }
    return false;
}

bool apply_input_binding_setting(
    ActionInputBindings& bindings,
    ControlBindings& controls,
    const std::string_view key,
    const std::string_view value) {
    InputBindingLoadState load_state {};
    return apply_input_binding_setting(bindings, controls, load_state, key, value);
}

void write_input_binding_settings(
    std::ostream& output,
    const ControlBindings& controls,
    const ActionInputBindings& bindings) {
    (void)controls;
    output << "p1_up_key=" << legacy_key_from_keyboard_scancode(bindings.p1_move_up_key) << "\n";
    output << "p1_down_key=" << legacy_key_from_keyboard_scancode(bindings.p1_move_down_key) << "\n";
    output << "p2_up_key=" << legacy_key_from_keyboard_scancode(bindings.p2_move_up_key) << "\n";
    output << "p2_down_key=" << legacy_key_from_keyboard_scancode(bindings.p2_move_down_key) << "\n";
    output << "p1_up_scancode=" << bindings.p1_move_up_key << "\n";
    output << "p1_down_scancode=" << bindings.p1_move_down_key << "\n";
    output << "p2_up_scancode=" << bindings.p2_move_up_key << "\n";
    output << "p2_down_scancode=" << bindings.p2_move_down_key << "\n";
    output << "p1_controller_index=" << bindings.p1_controller.controller_index << "\n";
    output << "p2_controller_index=" << bindings.p2_controller.controller_index << "\n";
    output << "p1_up_controller_button=" << static_cast<int>(bindings.p1_controller.move_up_button) << "\n";
    output << "p1_down_controller_button=" << static_cast<int>(bindings.p1_controller.move_down_button) << "\n";
    output << "p2_up_controller_button=" << static_cast<int>(bindings.p2_controller.move_up_button) << "\n";
    output << "p2_down_controller_button=" << static_cast<int>(bindings.p2_controller.move_down_button) << "\n";
}

void sync_controls_from_action_bindings(
    ControlBindings& controls,
    const ActionInputBindings& bindings) {
    controls.p1_up = bindings.p1_move_up_key;
    controls.p1_down = bindings.p1_move_down_key;
    controls.p2_up = bindings.p2_move_up_key;
    controls.p2_down = bindings.p2_move_down_key;
}

}  // namespace whacker::app
