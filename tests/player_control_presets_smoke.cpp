#include "action_input.hpp"
#include "player_control_presets.hpp"
#include "test_assert.hpp"

#include <cstddef>

namespace {

void set_controller_axis(
    whacker::app::InputPhysicalState& state,
    const int controller_index,
    const whacker::app::ControllerAxis axis,
    const float value) {
    state.controllers[static_cast<std::size_t>(controller_index)].connected = true;
    state.controllers[static_cast<std::size_t>(controller_index)]
        .axes[static_cast<std::size_t>(axis)] = value;
}

void set_controller_button(
    whacker::app::InputPhysicalState& state,
    const int controller_index,
    const whacker::app::ControllerButton button) {
    state.controllers[static_cast<std::size_t>(controller_index)].connected = true;
    state.controllers[static_cast<std::size_t>(controller_index)]
        .buttons[static_cast<std::size_t>(button)] = true;
}

void set_keyboard_scancode(
    whacker::app::InputPhysicalState& state,
    const int scancode) {
    state.keyboard.scancodes[static_cast<std::size_t>(scancode)] = true;
}

void test_default_layout_detects_separate_controllers() {
    const whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    TEST_CHECK(
        whacker::app::detect_player_control_preset(bindings) ==
        whacker::app::PlayerControlPreset::SeparateControllers);
}

void test_shared_keyboard_preset_removes_player_controller_movement() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    TEST_CHECK(whacker::app::apply_player_control_preset(
        bindings,
        whacker::app::PlayerControlPreset::SharedKeyboard));
    TEST_CHECK(
        whacker::app::detect_player_control_preset(bindings) ==
        whacker::app::PlayerControlPreset::SharedKeyboard);

    whacker::app::InputPhysicalState keyboard {};
    set_keyboard_scancode(keyboard, whacker::app::kKeyboardScancodeW);
    set_keyboard_scancode(keyboard, whacker::app::kKeyboardScancodeDown);
    whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, keyboard, bindings);
    TEST_CHECK(frame.p1_move_y == -1.0f);
    TEST_CHECK(frame.p2_move_y == 1.0f);

    whacker::app::InputPhysicalState controller {};
    set_controller_axis(controller, 0, whacker::app::ControllerAxis::LeftY, -1.0f);
    set_controller_axis(controller, 1, whacker::app::ControllerAxis::LeftY, 1.0f);
    frame = whacker::app::derive_action_input_frame({}, controller, bindings);
    TEST_CHECK(frame.p1_move_y == 0.0f);
    TEST_CHECK(frame.p2_move_y == 0.0f);
}

void test_shared_controller_preset_drives_both_slots_from_controller_zero() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    TEST_CHECK(whacker::app::apply_player_control_preset(
        bindings,
        whacker::app::PlayerControlPreset::SharedController));
    TEST_CHECK(
        whacker::app::detect_player_control_preset(bindings) ==
        whacker::app::PlayerControlPreset::SharedController);

    whacker::app::InputPhysicalState current {};
    set_controller_axis(current, 0, whacker::app::ControllerAxis::LeftY, -0.60f);
    set_controller_axis(current, 0, whacker::app::ControllerAxis::RightY, 0.80f);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);
    TEST_CHECK(frame.p1_move_y == -0.60f);
    TEST_CHECK(frame.p2_move_y == 0.80f);
}

void test_shared_controller_button_fallbacks_are_distinct() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    (void)whacker::app::apply_player_control_preset(
        bindings,
        whacker::app::PlayerControlPreset::SharedController);

    whacker::app::InputPhysicalState current {};
    set_controller_button(current, 0, whacker::app::ControllerButton::DpadDown);
    set_controller_button(current, 0, whacker::app::ControllerButton::Y);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);
    TEST_CHECK(frame.p1_move_y == 1.0f);
    TEST_CHECK(frame.p2_move_y == -1.0f);
}

void test_keyboard_vs_controller_preset_separates_slots() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    TEST_CHECK(whacker::app::apply_player_control_preset(
        bindings,
        whacker::app::PlayerControlPreset::KeyboardVsController));
    TEST_CHECK(
        whacker::app::detect_player_control_preset(bindings) ==
        whacker::app::PlayerControlPreset::KeyboardVsController);

    whacker::app::InputPhysicalState current {};
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeS);
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeDown);
    set_controller_axis(current, 0, whacker::app::ControllerAxis::LeftY, -0.75f);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);
    TEST_CHECK(frame.p1_move_y == 1.0f);
    TEST_CHECK(frame.p2_move_y == -0.75f);
}

void test_manual_rebind_detection_becomes_custom() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::bind_player_move_axis(
        bindings,
        whacker::app::InputSlot::P2,
        whacker::app::ControllerAxis::RightX,
        false);
    TEST_CHECK(
        whacker::app::detect_player_control_preset(bindings) ==
        whacker::app::PlayerControlPreset::Custom);
}

void test_cycle_from_custom_lands_on_selectable_preset() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::bind_player_move_axis(
        bindings,
        whacker::app::InputSlot::P2,
        whacker::app::ControllerAxis::RightX,
        false);

    TEST_CHECK(whacker::app::cycle_player_control_preset(bindings, 1));
    TEST_CHECK(
        whacker::app::detect_player_control_preset(bindings) ==
        whacker::app::PlayerControlPreset::SharedKeyboard);
}

}  // namespace

int main() {
    test_default_layout_detects_separate_controllers();
    test_shared_keyboard_preset_removes_player_controller_movement();
    test_shared_controller_preset_drives_both_slots_from_controller_zero();
    test_shared_controller_button_fallbacks_are_distinct();
    test_keyboard_vs_controller_preset_separates_slots();
    test_manual_rebind_detection_becomes_custom();
    test_cycle_from_custom_lands_on_selectable_preset();
    return 0;
}
