#include "action_input.hpp"
#include "input_profiles.hpp"
#include "test_assert.hpp"

#include <cstddef>
#include <string>

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

void test_desktop_defaults_stay_two_controller_ready() {
    whacker::app::InputPhysicalState current {};
    set_controller_axis(current, 0, whacker::app::ControllerAxis::LeftY, -0.45f);
    set_controller_axis(current, 1, whacker::app::ControllerAxis::LeftY, 0.70f);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, whacker::app::default_action_input_bindings());

    TEST_CHECK(frame.p1_move_y == -0.45f);
    TEST_CHECK(frame.p2_move_y == 0.70f);
}

void test_handheld_controller_zero_drives_menu_actions() {
    const whacker::app::ActionInputBindings bindings = whacker::app::handheld_action_input_bindings();

    whacker::app::InputPhysicalState current {};
    set_controller_button(current, 0, whacker::app::ControllerButton::DpadDown);
    set_controller_axis(current, 0, whacker::app::ControllerAxis::LeftX, 0.80f);
    set_controller_button(current, 0, whacker::app::ControllerButton::A);
    set_controller_button(current, 0, whacker::app::ControllerButton::B);
    set_controller_button(current, 0, whacker::app::ControllerButton::Start);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);

    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::MenuRight));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::Confirm));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::Back));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::Pause));
}

void test_handheld_select_button_is_not_ui_back() {
    const whacker::app::ActionInputBindings bindings = whacker::app::handheld_action_input_bindings();

    whacker::app::InputPhysicalState select_current {};
    set_controller_button(select_current, 0, whacker::app::ControllerButton::Back);

    const whacker::app::ActionInputFrame select_frame =
        whacker::app::derive_action_input_frame({}, select_current, bindings);

    TEST_CHECK(!whacker::app::input_pressed(select_frame, whacker::app::InputAction::Back));

    whacker::app::InputPhysicalState face_current {};
    set_controller_button(face_current, 0, whacker::app::ControllerButton::B);

    const whacker::app::ActionInputFrame face_frame =
        whacker::app::derive_action_input_frame({}, face_current, bindings);

    TEST_CHECK(whacker::app::input_pressed(face_frame, whacker::app::InputAction::Back));
}

void test_handheld_controller_zero_drives_story_player_slot() {
    const whacker::app::ActionInputBindings bindings = whacker::app::handheld_action_input_bindings();

    whacker::app::InputPhysicalState stick_current {};
    set_controller_axis(stick_current, 0, whacker::app::ControllerAxis::LeftY, -0.65f);
    set_controller_axis(stick_current, 1, whacker::app::ControllerAxis::LeftY, 0.55f);

    const whacker::app::ActionInputFrame stick_frame =
        whacker::app::derive_action_input_frame({}, stick_current, bindings);

    TEST_CHECK(stick_frame.p1_move_y == -0.65f);
    TEST_CHECK(stick_frame.p2_move_y == 0.55f);

    whacker::app::InputPhysicalState dpad_current {};
    set_controller_button(dpad_current, 0, whacker::app::ControllerButton::DpadDown);

    const whacker::app::ActionInputFrame dpad_frame =
        whacker::app::derive_action_input_frame({}, dpad_current, bindings);

    TEST_CHECK(dpad_frame.p1_move_y == 1.0f);
    TEST_CHECK(dpad_frame.p2_move_y == 0.0f);
}

void test_handheld_preset_replaces_previous_controller_indices() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::bind_menu_controller(bindings, 2);
    whacker::app::bind_menu_axes(
        bindings,
        whacker::app::ControllerAxis::RightX,
        whacker::app::ControllerAxis::RightY);
    whacker::app::bind_player_controller(bindings, whacker::app::InputSlot::P1, 2);

    whacker::app::apply_handheld_action_input_preset(bindings);
    const std::size_t binding_count = bindings.bindings.size();
    whacker::app::apply_handheld_action_input_preset(bindings);
    TEST_CHECK(bindings.bindings.size() == binding_count);

    whacker::app::InputPhysicalState current {};
    set_controller_button(current, 2, whacker::app::ControllerButton::A);
    set_controller_axis(current, 2, whacker::app::ControllerAxis::LeftY, -1.0f);
    set_controller_axis(current, 0, whacker::app::ControllerAxis::RightX, 1.0f);

    whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::Confirm));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::MenuRight));
    TEST_CHECK(frame.p1_move_y == 0.0f);

    whacker::app::InputPhysicalState handheld_current {};
    set_controller_button(handheld_current, 0, whacker::app::ControllerButton::A);
    set_controller_axis(handheld_current, 0, whacker::app::ControllerAxis::LeftX, 1.0f);
    set_controller_axis(handheld_current, 0, whacker::app::ControllerAxis::LeftY, -1.0f);

    frame = whacker::app::derive_action_input_frame({}, handheld_current, bindings);
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::Confirm));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::MenuRight));
    TEST_CHECK(frame.p1_move_y == -1.0f);
}

void test_profile_helpers_select_expected_bindings() {
    TEST_CHECK(whacker::app::input_profile_name(whacker::app::InputProfile::Desktop) == std::string("desktop"));
    TEST_CHECK(whacker::app::input_profile_name(whacker::app::InputProfile::Handheld) == std::string("handheld"));

    whacker::app::InputPhysicalState current {};
    set_controller_axis(current, 1, whacker::app::ControllerAxis::LeftY, 0.5f);

    const whacker::app::ActionInputFrame desktop_frame =
        whacker::app::derive_action_input_frame(
            {},
            current,
            whacker::app::action_input_bindings_for_profile(whacker::app::InputProfile::Desktop));
    const whacker::app::ActionInputFrame handheld_frame =
        whacker::app::derive_action_input_frame(
            {},
            current,
            whacker::app::action_input_bindings_for_profile(whacker::app::InputProfile::Handheld));

    TEST_CHECK(desktop_frame.p2_move_y == 0.5f);
    TEST_CHECK(handheld_frame.p2_move_y == 0.5f);
}

}  // namespace

int main() {
    test_desktop_defaults_stay_two_controller_ready();
    test_handheld_controller_zero_drives_menu_actions();
    test_handheld_select_button_is_not_ui_back();
    test_handheld_controller_zero_drives_story_player_slot();
    test_handheld_preset_replaces_previous_controller_indices();
    test_profile_helpers_select_expected_bindings();
    return 0;
}
