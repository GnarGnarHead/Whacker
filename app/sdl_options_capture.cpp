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

bool set_controller_axis_for_options_row(
    ActionInputBindings& bindings,
    const int row,
    const ControllerAxis axis) {
    InputSlot slot = InputSlot::P1;
    if (!sdl_options_axis_row(row, slot)) {
        return false;
    }
    bind_player_move_axis(
        bindings,
        slot,
        axis,
        controller_axis_inverted_for_input_slot(bindings, slot));
    return true;
}

bool set_controller_index_for_options_row(
    ActionInputBindings& bindings,
    const int row,
    const int controller_index) {
    SdlOptionsBindingRow binding_row {};
    if (sdl_options_binding_row(row, binding_row)) {
        bind_controller_index_for_input_slot(bindings, binding_row.slot, controller_index);
        return true;
    }
    InputSlot slot = InputSlot::P1;
    if (sdl_options_axis_row(row, slot) || sdl_options_axis_invert_row(row, slot)) {
        bind_controller_index_for_input_slot(bindings, slot, controller_index);
        return true;
    }
    return false;
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

    InputSlot axis_slot = InputSlot::P1;
    const bool capturing_axis = sdl_options_axis_row(selected_row, axis_slot);

    if (events.keyboard_key_pressed) {
        if (events.keyboard_scancode == kKeyboardScancodeEscape) {
            result.finished = true;
        } else if (!capturing_axis && sdl_keyboard_scancode_bindable(events.keyboard_scancode)) {
            result.binding_changed =
                set_keyboard_scancode_for_options_row(bindings, selected_row, events.keyboard_scancode);
            if (result.binding_changed) {
                sync_controls_from_action_bindings(controls, bindings);
            }
            result.finished = true;
        }
    }

    if (!result.finished && !capturing_axis && events.controller_button_pressed) {
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

    if (!result.finished && capturing_axis && events.controller_axis_moved) {
        if (set_controller_axis_for_options_row(bindings, selected_row, events.controller_axis)) {
            const int controller_index = input.controller_index_for_instance_id(events.controller_axis_instance_id);
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

bool cycle_sdl_options_controller_axis(
    ActionInputBindings& bindings,
    const int row,
    const int direction) {
    if (direction == 0) {
        return false;
    }
    InputSlot slot = InputSlot::P1;
    if (!sdl_options_axis_row(row, slot)) {
        return false;
    }
    const ControllerAxis current = controller_axis_for_input_slot(bindings, slot);
    const ControllerAxis next = next_bindable_controller_axis(current, direction);
    bind_player_move_axis(
        bindings,
        slot,
        next,
        controller_axis_inverted_for_input_slot(bindings, slot));
    return next != current;
}

bool toggle_sdl_options_controller_axis_invert(
    ActionInputBindings& bindings,
    const int row) {
    InputSlot slot = InputSlot::P1;
    if (!sdl_options_axis_invert_row(row, slot)) {
        return false;
    }
    bind_player_move_axis(
        bindings,
        slot,
        controller_axis_for_input_slot(bindings, slot),
        !controller_axis_inverted_for_input_slot(bindings, slot));
    return true;
}

}  // namespace whacker::app
