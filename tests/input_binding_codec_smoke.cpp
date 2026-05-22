#include "input_binding_codec.hpp"
#include "test_assert.hpp"

#include <sstream>
#include <string>

namespace {

bool contains(const std::string& haystack, const char* needle) {
    return haystack.find(needle) != std::string::npos;
}

void test_legacy_key_values_migrate_to_scancodes() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::ControlHintBindings controls {};

    TEST_CHECK(whacker::app::keyboard_scancode_from_legacy_key(265) == whacker::app::kKeyboardScancodeUp);
    TEST_CHECK(whacker::app::legacy_key_from_keyboard_scancode(whacker::app::kKeyboardScancodeW) == 'W');
    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, "p2_up_key", "265"));

    TEST_CHECK(whacker::app::keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P2,
        whacker::app::AxisDirection::Negative) == whacker::app::kKeyboardScancodeUp);
    TEST_CHECK(controls.p2_up == whacker::app::kKeyboardScancodeUp);
}

void test_scancode_keys_override_legacy_keys() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::ControlHintBindings controls {};
    whacker::app::InputBindingLoadState load_state {};

    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, load_state, "p1_up_key", "87"));
    TEST_CHECK(whacker::app::keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative) == whacker::app::kKeyboardScancodeW);

    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, load_state, "p1_up_scancode", "82"));
    TEST_CHECK(whacker::app::keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative) == whacker::app::kKeyboardScancodeUp);
    TEST_CHECK(controls.p1_up == whacker::app::kKeyboardScancodeUp);
}

void test_scancode_keys_block_later_legacy_keys_in_same_file() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::ControlHintBindings controls {};
    whacker::app::InputBindingLoadState load_state {};

    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, load_state, "p1_up_scancode", "82"));
    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, load_state, "p1_up_key", "87"));

    TEST_CHECK(whacker::app::keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative) == whacker::app::kKeyboardScancodeUp);
    TEST_CHECK(controls.p1_up == whacker::app::kKeyboardScancodeUp);
}

void test_scancode_precedence_resets_across_files() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::ControlHintBindings controls {};
    whacker::app::InputBindingLoadState first_file {};
    whacker::app::InputBindingLoadState second_file {};

    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, first_file, "p1_up_scancode", "82"));
    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, second_file, "p1_up_key", "87"));

    TEST_CHECK(whacker::app::keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative) == whacker::app::kKeyboardScancodeW);
    TEST_CHECK(controls.p1_up == whacker::app::kKeyboardScancodeW);
}

void test_invalid_binding_values_do_not_mutate() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::ControlHintBindings controls {};
    const int original_key = whacker::app::keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative);
    const whacker::app::ControllerButton original_button =
        whacker::app::controller_button_for_move_direction(
            bindings,
            whacker::app::InputSlot::P1,
            whacker::app::AxisDirection::Negative);

    TEST_CHECK(!whacker::app::apply_input_binding_setting(bindings, controls, "p1_up_scancode", "-1"));
    TEST_CHECK(!whacker::app::apply_input_binding_setting(bindings, controls, "p1_up_controller_button", "999"));

    TEST_CHECK(whacker::app::keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative) == original_key);
    TEST_CHECK(whacker::app::controller_button_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative) == original_button);
}

void test_controller_index_is_clamped() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::ControlHintBindings controls {};

    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, "p1_controller_index", "-8"));
    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, "p2_controller_index", "99"));

    TEST_CHECK(whacker::app::controller_index_for_input_slot(bindings, whacker::app::InputSlot::P1) == 0);
    TEST_CHECK(
        whacker::app::controller_index_for_input_slot(bindings, whacker::app::InputSlot::P2) ==
        whacker::app::kMaxInputControllers - 1);
}

void test_writer_emits_binding_keys() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::ControlHintBindings controls {};
    TEST_CHECK(whacker::app::bind_keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative,
        whacker::app::kKeyboardScancodeUp));
    whacker::app::bind_controller_index_for_input_slot(bindings, whacker::app::InputSlot::P1, 2);
    TEST_CHECK(whacker::app::bind_controller_button_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative,
        whacker::app::ControllerButton::RightShoulder));

    std::ostringstream output;
    whacker::app::write_input_binding_settings(output, controls, bindings);
    const std::string text = output.str();

    TEST_CHECK(!contains(text, "p1_up_key="));
    TEST_CHECK(!contains(text, "p1_down_key="));
    TEST_CHECK(!contains(text, "p2_up_key="));
    TEST_CHECK(!contains(text, "p2_down_key="));
    TEST_CHECK(contains(text, "p1_up_scancode=82\n"));
    TEST_CHECK(contains(text, "p2_down_scancode=81\n"));
    TEST_CHECK(contains(text, "p1_controller_index=2\n"));
    TEST_CHECK(contains(text, "p2_controller_index=1\n"));
    TEST_CHECK(contains(text, "p1_controller_axis=1\n"));
    TEST_CHECK(contains(text, "p2_controller_axis=1\n"));
    TEST_CHECK(contains(text, "p1_controller_axis_invert=0\n"));
    TEST_CHECK(contains(text, "p2_controller_axis_invert=0\n"));
    TEST_CHECK(contains(text, "p1_up_controller_button=10\n"));
    TEST_CHECK(contains(text, "p2_down_controller_button=12\n"));
}

void test_controller_axis_settings_round_trip_through_generic_bindings() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::ControlHintBindings controls {};

    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, "p1_controller_axis", "3"));
    TEST_CHECK(whacker::app::apply_input_binding_setting(bindings, controls, "p1_controller_axis_invert", "1"));

    TEST_CHECK(
        whacker::app::controller_axis_for_input_slot(bindings, whacker::app::InputSlot::P1) ==
        whacker::app::ControllerAxis::RightY);
    TEST_CHECK(whacker::app::controller_axis_inverted_for_input_slot(bindings, whacker::app::InputSlot::P1));
}

void test_sync_controls_from_action_bindings() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::ControlHintBindings controls {};
    TEST_CHECK(whacker::app::bind_keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Negative,
        whacker::app::kKeyboardScancodeUp));
    TEST_CHECK(whacker::app::bind_keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::AxisDirection::Positive,
        whacker::app::kKeyboardScancodeDown));
    TEST_CHECK(whacker::app::bind_keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P2,
        whacker::app::AxisDirection::Negative,
        whacker::app::kKeyboardScancodeW));
    TEST_CHECK(whacker::app::bind_keyboard_scancode_for_move_direction(
        bindings,
        whacker::app::InputSlot::P2,
        whacker::app::AxisDirection::Positive,
        whacker::app::kKeyboardScancodeS));

    whacker::app::sync_controls_from_action_bindings(controls, bindings);

    TEST_CHECK(controls.p1_up == whacker::app::kKeyboardScancodeUp);
    TEST_CHECK(controls.p1_down == whacker::app::kKeyboardScancodeDown);
    TEST_CHECK(controls.p2_up == whacker::app::kKeyboardScancodeW);
    TEST_CHECK(controls.p2_down == whacker::app::kKeyboardScancodeS);
}

}  // namespace

int main() {
    test_legacy_key_values_migrate_to_scancodes();
    test_scancode_keys_override_legacy_keys();
    test_scancode_keys_block_later_legacy_keys_in_same_file();
    test_scancode_precedence_resets_across_files();
    test_invalid_binding_values_do_not_mutate();
    test_controller_index_is_clamped();
    test_writer_emits_binding_keys();
    test_controller_axis_settings_round_trip_through_generic_bindings();
    test_sync_controls_from_action_bindings();
    return 0;
}
