#include "sdl_options_capture.hpp"

#include "input_binding_codec.hpp"
#include "sdl_input.hpp"
#include "sdl_options_binding_access.hpp"

namespace whacker::app {

namespace {

bool set_keyboard_scancode_for_options_row(ActionInputBindings& bindings, const int row, const int scancode) {
    SdlOptionsBindingRow binding_row {};
    if (!sdl_options_binding_row(row, binding_row)) {
        return false;
    }
    return bind_keyboard_scancode_for_move_direction(
        bindings,
        binding_row.slot,
        binding_row.direction,
        scancode);
}

bool set_controller_button_for_options_row(
    ActionInputBindings& bindings,
    const int row,
    const ControllerButton button) {
    SdlOptionsBindingRow binding_row {};
    if (!sdl_options_binding_row(row, binding_row)) {
        return false;
    }
    return bind_controller_button_for_move_direction(
        bindings,
        binding_row.slot,
        binding_row.direction,
        button);
}

bool set_controller_index_for_options_row(
    ActionInputBindings& bindings,
    const int row,
    const int controller_index) {
    SdlOptionsBindingRow binding_row {};
    if (!sdl_options_binding_row(row, binding_row)) {
        return false;
    }
    bind_controller_index_for_input_slot(bindings, binding_row.slot, controller_index);
    return true;
}

}  // namespace

SdlOptionsCaptureResult apply_sdl_options_capture(
    OptionsMenuState& options_menu_state,
    ActionInputBindings& bindings,
    ControlHintBindings& controls,
    const SdlInput& input,
    const SdlEventFrame& events) {
    SdlOptionsCaptureResult result {};
    const int selected_row = options_menu_state.selected_row;

    if (events.keyboard_key_pressed) {
        if (events.keyboard_scancode == kKeyboardScancodeEscape) {
            result.finished = true;
        } else if (sdl_keyboard_scancode_bindable(events.keyboard_scancode)) {
            result.binding_changed =
                set_keyboard_scancode_for_options_row(bindings, selected_row, events.keyboard_scancode);
            if (result.binding_changed) {
                sync_controls_from_action_bindings(controls, bindings);
            }
            result.finished = true;
        }
    }

    if (!result.finished && events.controller_button_pressed) {
        if (events.controller_button != ControllerButton::Unbound &&
            set_controller_button_for_options_row(bindings, selected_row, events.controller_button)) {
            const int controller_index = input.controller_index_for_instance_id(events.controller_instance_id);
            if (controller_index >= 0) {
                (void)set_controller_index_for_options_row(bindings, selected_row, controller_index);
            }
            result.binding_changed = true;
        }
        result.finished = true;
    }

    if (result.finished) {
        options_menu_state.waiting_for_key = false;
    }
    return result;
}

bool cycle_sdl_options_controller_button(
    ActionInputBindings& bindings,
    const int row,
    const int direction) {
    if (direction == 0) {
        return false;
    }
    const ControllerButton current = controller_button_for_options_row(bindings, row);
    const ControllerButton next = next_bindable_controller_button(current, direction);
    return set_controller_button_for_options_row(bindings, row, next);
}

}  // namespace whacker::app
