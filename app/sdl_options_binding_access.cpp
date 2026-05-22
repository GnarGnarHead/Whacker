#include "sdl_options_binding_access.hpp"

#include <array>
#include <cstddef>

#include "ui_state.hpp"

namespace whacker::app {

ControllerButton controller_button_for_options_row(const ActionInputBindings& bindings, const int row) {
    switch (row) {
        case OptionsMenuRowP1Up:
            return bindings.p1_controller.move_up_button;
        case OptionsMenuRowP1Down:
            return bindings.p1_controller.move_down_button;
        case OptionsMenuRowP2Up:
            return bindings.p2_controller.move_up_button;
        case OptionsMenuRowP2Down:
            return bindings.p2_controller.move_down_button;
        default:
            return ControllerButton::Unbound;
    }
}

int keyboard_scancode_for_options_row(const ActionInputBindings& bindings, const int row) {
    switch (row) {
        case OptionsMenuRowP1Up:
            return bindings.p1_move_up_key;
        case OptionsMenuRowP1Down:
            return bindings.p1_move_down_key;
        case OptionsMenuRowP2Up:
            return bindings.p2_move_up_key;
        case OptionsMenuRowP2Down:
            return bindings.p2_move_down_key;
        default:
            return kKeyboardScancodeUnbound;
    }
}

int controller_index_for_options_row(const ActionInputBindings& bindings, const int row) {
    switch (row) {
        case OptionsMenuRowP1Up:
        case OptionsMenuRowP1Down:
            return bindings.p1_controller.controller_index;
        case OptionsMenuRowP2Up:
        case OptionsMenuRowP2Down:
            return bindings.p2_controller.controller_index;
        default:
            return 0;
    }
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

}  // namespace whacker::app
