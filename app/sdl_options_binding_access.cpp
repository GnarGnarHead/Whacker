#include "sdl_options_binding_access.hpp"

#include <array>
#include <cstddef>

#include "ui_state.hpp"

namespace whacker::app {

namespace {

InputSlot input_slot_for_options_row(const int row) {
    switch (row) {
        case OptionsControlsRowP1Up:
        case OptionsControlsRowP1Down:
        case OptionsControlsRowP1Axis:
        case OptionsControlsRowP1AxisInvert:
            return InputSlot::P1;
        case OptionsControlsRowP2Up:
        case OptionsControlsRowP2Down:
        case OptionsControlsRowP2Axis:
        case OptionsControlsRowP2AxisInvert:
            return InputSlot::P2;
        default:
            return InputSlot::P1;
    }
}

AxisDirection axis_direction_for_options_row(const int row) {
    switch (row) {
        case OptionsControlsRowP1Up:
        case OptionsControlsRowP2Up:
            return AxisDirection::Negative;
        case OptionsControlsRowP1Down:
        case OptionsControlsRowP2Down:
            return AxisDirection::Positive;
        default:
            return AxisDirection::Negative;
    }
}

bool options_row_has_input_binding(const int row) {
    switch (row) {
        case OptionsControlsRowP1Up:
        case OptionsControlsRowP1Down:
        case OptionsControlsRowP2Up:
        case OptionsControlsRowP2Down:
            return true;
        default:
            return false;
    }
}

bool options_row_has_axis_binding(const int row) {
    return row == OptionsControlsRowP1Axis || row == OptionsControlsRowP2Axis;
}

bool options_row_has_axis_invert(const int row) {
    return row == OptionsControlsRowP1AxisInvert || row == OptionsControlsRowP2AxisInvert;
}

}  // namespace

bool sdl_options_binding_row(
    const OptionsMenuSection section,
    const int row,
    SdlOptionsBindingRow& binding_row) {
    if (section != OptionsMenuSection::Controls || !options_row_has_input_binding(row)) {
        return false;
    }
    binding_row = SdlOptionsBindingRow {
        .slot = input_slot_for_options_row(row),
        .direction = axis_direction_for_options_row(row),
    };
    return true;
}

bool sdl_options_axis_row(const OptionsMenuSection section, const int row, InputSlot& slot) {
    if (section != OptionsMenuSection::Controls || !options_row_has_axis_binding(row)) {
        return false;
    }
    slot = input_slot_for_options_row(row);
    return true;
}

bool sdl_options_axis_invert_row(const OptionsMenuSection section, const int row, InputSlot& slot) {
    if (section != OptionsMenuSection::Controls || !options_row_has_axis_invert(row)) {
        return false;
    }
    slot = input_slot_for_options_row(row);
    return true;
}

ControllerButton controller_button_for_options_row(
    const ActionInputBindings& bindings,
    const OptionsMenuSection section,
    const int row) {
    SdlOptionsBindingRow binding_row {};
    if (!sdl_options_binding_row(section, row, binding_row)) {
        return ControllerButton::Unbound;
    }
    return controller_button_for_move_direction(
        bindings,
        binding_row.slot,
        binding_row.direction);
}

int keyboard_scancode_for_options_row(
    const ActionInputBindings& bindings,
    const OptionsMenuSection section,
    const int row) {
    SdlOptionsBindingRow binding_row {};
    if (!sdl_options_binding_row(section, row, binding_row)) {
        return kKeyboardScancodeUnbound;
    }
    return keyboard_scancode_for_move_direction(
        bindings,
        binding_row.slot,
        binding_row.direction);
}

int controller_index_for_options_row(
    const ActionInputBindings& bindings,
    const OptionsMenuSection section,
    const int row) {
    SdlOptionsBindingRow binding_row {};
    if (sdl_options_binding_row(section, row, binding_row)) {
        return controller_index_for_input_slot(bindings, binding_row.slot);
    }
    InputSlot slot = InputSlot::P1;
    if (sdl_options_axis_row(section, row, slot) || sdl_options_axis_invert_row(section, row, slot)) {
        return controller_index_for_input_slot(bindings, slot);
    }
    return 0;
}

ControllerAxis controller_axis_for_options_row(
    const ActionInputBindings& bindings,
    const OptionsMenuSection section,
    const int row) {
    InputSlot slot = InputSlot::P1;
    if (!sdl_options_axis_row(section, row, slot) && !sdl_options_axis_invert_row(section, row, slot)) {
        return ControllerAxis::LeftY;
    }
    return controller_axis_for_input_slot(bindings, slot);
}

bool controller_axis_inverted_for_options_row(
    const ActionInputBindings& bindings,
    const OptionsMenuSection section,
    const int row) {
    InputSlot slot = InputSlot::P1;
    if (!sdl_options_axis_row(section, row, slot) && !sdl_options_axis_invert_row(section, row, slot)) {
        return false;
    }
    return controller_axis_inverted_for_input_slot(bindings, slot);
}

ControllerButton next_bindable_controller_button(const ControllerButton current, const int direction) {
    constexpr std::array<ControllerButton, 12> kButtons {{
        ControllerButton::DpadUp,
        ControllerButton::DpadDown,
        ControllerButton::DpadLeft,
        ControllerButton::DpadRight,
        ControllerButton::A,
        ControllerButton::B,
        ControllerButton::X,
        ControllerButton::Y,
        ControllerButton::LeftShoulder,
        ControllerButton::RightShoulder,
        ControllerButton::Start,
        ControllerButton::Back,
    }};
    if (direction == 0) {
        return current;
    }
    int index = 0;
    for (int i = 0; i < static_cast<int>(kButtons.size()); ++i) {
        if (kButtons[static_cast<std::size_t>(i)] == current) {
            index = i;
            break;
        }
    }
    index = (index + direction + static_cast<int>(kButtons.size())) % static_cast<int>(kButtons.size());
    return kButtons[static_cast<std::size_t>(index)];
}

ControllerAxis next_bindable_controller_axis(const ControllerAxis current, const int direction) {
    constexpr std::array<ControllerAxis, 4> kAxes {{
        ControllerAxis::LeftY,
        ControllerAxis::RightY,
        ControllerAxis::LeftX,
        ControllerAxis::RightX,
    }};
    if (direction == 0) {
        return current;
    }
    int index = 0;
    for (int i = 0; i < static_cast<int>(kAxes.size()); ++i) {
        if (kAxes[static_cast<std::size_t>(i)] == current) {
            index = i;
            break;
        }
    }
    index = (index + direction + static_cast<int>(kAxes.size())) % static_cast<int>(kAxes.size());
    return kAxes[static_cast<std::size_t>(index)];
}

}  // namespace whacker::app
