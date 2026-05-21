#include "action_input.hpp"
#include "main_menu_actions.hpp"
#include "test_assert.hpp"

namespace {

void test_keyboard_edges_and_menu_actions() {
    whacker::app::KeyboardPhysicalState previous {};
    whacker::app::KeyboardPhysicalState current {};
    current.key_w = true;
    current.key_enter = true;

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_keyboard_action_frame(previous, current);

    TEST_CHECK(whacker::app::input_held(frame, whacker::app::InputAction::MenuUp));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::MenuUp));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::Confirm));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(frame.p1_move_y == -1.0f);
    TEST_CHECK(frame.p2_move_y == 0.0f);
}

void test_held_keys_are_not_repeated_presses() {
    whacker::app::KeyboardPhysicalState previous {};
    previous.key_down = true;
    whacker::app::KeyboardPhysicalState current {};
    current.key_down = true;

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_keyboard_action_frame(previous, current);

    TEST_CHECK(whacker::app::input_held(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(!whacker::app::input_released(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(frame.p2_move_y == 1.0f);
}

void test_released_key_reports_release_edge() {
    whacker::app::KeyboardPhysicalState previous {};
    previous.key_escape = true;
    whacker::app::KeyboardPhysicalState current {};

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_keyboard_action_frame(previous, current);

    TEST_CHECK(!whacker::app::input_held(frame, whacker::app::InputAction::Pause));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::Pause));
    TEST_CHECK(whacker::app::input_released(frame, whacker::app::InputAction::Pause));
}

void test_opposed_axis_cancels_to_zero() {
    whacker::app::KeyboardPhysicalState current {};
    current.key_w = true;
    current.key_s = true;
    current.key_up = true;
    current.key_down = true;

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_keyboard_action_frame({}, current);

    TEST_CHECK(frame.p1_move_y == 0.0f);
    TEST_CHECK(frame.p2_move_y == 0.0f);
}

void test_main_menu_confirm_maps_every_visible_row() {
    whacker::app::MainMenuState menu_state {};

    menu_state.selected_row = whacker::app::MainMenuRowStory;
    TEST_CHECK(
        whacker::app::apply_main_menu_action(menu_state, false, false, true, false) ==
        whacker::app::MainMenuActionResult::Story);

    menu_state.selected_row = whacker::app::MainMenuRowQuick;
    TEST_CHECK(
        whacker::app::apply_main_menu_action(menu_state, false, false, true, false) ==
        whacker::app::MainMenuActionResult::Quick);

    menu_state.selected_row = whacker::app::MainMenuRowOptions;
    TEST_CHECK(
        whacker::app::apply_main_menu_action(menu_state, false, false, true, false) ==
        whacker::app::MainMenuActionResult::Options);

    menu_state.selected_row = whacker::app::MainMenuRowQuit;
    TEST_CHECK(
        whacker::app::apply_main_menu_action(menu_state, false, false, true, false) ==
        whacker::app::MainMenuActionResult::Quit);
}

}  // namespace

int main() {
    test_keyboard_edges_and_menu_actions();
    test_held_keys_are_not_repeated_presses();
    test_released_key_reports_release_edge();
    test_opposed_axis_cancels_to_zero();
    test_main_menu_confirm_maps_every_visible_row();
    return 0;
}
