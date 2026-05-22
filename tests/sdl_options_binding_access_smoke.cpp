#include "sdl_options_binding_access.hpp"
#include "test_assert.hpp"
#include "ui_state.hpp"

namespace {

void test_direction_rows_still_describe_button_bindings() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::SdlOptionsBindingRow row {};

    TEST_CHECK(whacker::app::sdl_options_binding_row(whacker::app::OptionsMenuRowP1Up, row));
    TEST_CHECK(row.slot == whacker::app::InputSlot::P1);
    TEST_CHECK(row.direction == whacker::app::AxisDirection::Negative);
    TEST_CHECK(
        whacker::app::controller_button_for_options_row(bindings, whacker::app::OptionsMenuRowP1Up) ==
        whacker::app::ControllerButton::DpadUp);

    TEST_CHECK(whacker::app::sdl_options_binding_row(whacker::app::OptionsMenuRowP2Down, row));
    TEST_CHECK(row.slot == whacker::app::InputSlot::P2);
    TEST_CHECK(row.direction == whacker::app::AxisDirection::Positive);
    TEST_CHECK(
        whacker::app::controller_button_for_options_row(bindings, whacker::app::OptionsMenuRowP2Down) ==
        whacker::app::ControllerButton::DpadDown);
}

void test_axis_rows_describe_analog_move_bindings() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::InputSlot slot = whacker::app::InputSlot::P1;

    TEST_CHECK(whacker::app::sdl_options_axis_row(whacker::app::OptionsMenuRowP1Axis, slot));
    TEST_CHECK(slot == whacker::app::InputSlot::P1);
    whacker::app::SdlOptionsBindingRow direction_row {};
    TEST_CHECK(!whacker::app::sdl_options_binding_row(whacker::app::OptionsMenuRowP1Axis, direction_row));
    TEST_CHECK(
        whacker::app::controller_axis_for_options_row(bindings, whacker::app::OptionsMenuRowP1Axis) ==
        whacker::app::ControllerAxis::LeftY);
    TEST_CHECK(!whacker::app::controller_axis_inverted_for_options_row(
        bindings,
        whacker::app::OptionsMenuRowP1AxisInvert));

    whacker::app::bind_player_move_axis(
        bindings,
        whacker::app::InputSlot::P2,
        whacker::app::ControllerAxis::RightX,
        true);
    TEST_CHECK(whacker::app::sdl_options_axis_row(whacker::app::OptionsMenuRowP2Axis, slot));
    TEST_CHECK(slot == whacker::app::InputSlot::P2);
    TEST_CHECK(
        whacker::app::controller_axis_for_options_row(bindings, whacker::app::OptionsMenuRowP2Axis) ==
        whacker::app::ControllerAxis::RightX);
    TEST_CHECK(whacker::app::controller_axis_inverted_for_options_row(
        bindings,
        whacker::app::OptionsMenuRowP2AxisInvert));
}

void test_axis_cycle_order_is_stable() {
    TEST_CHECK(
        whacker::app::next_bindable_controller_axis(whacker::app::ControllerAxis::LeftY, 1) ==
        whacker::app::ControllerAxis::RightY);
    TEST_CHECK(
        whacker::app::next_bindable_controller_axis(whacker::app::ControllerAxis::LeftY, -1) ==
        whacker::app::ControllerAxis::RightX);
}

}  // namespace

int main() {
    test_direction_rows_still_describe_button_bindings();
    test_axis_rows_describe_analog_move_bindings();
    test_axis_cycle_order_is_stable();
    return 0;
}
