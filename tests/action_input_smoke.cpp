#include "action_input.hpp"
#include "main_menu_actions.hpp"
#include "options_menu_actions.hpp"
#include "pause_menu_actions.hpp"
#include "quick_menu_actions.hpp"
#include "story_menu_actions.hpp"
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

void set_keyboard_scancode(
    whacker::app::KeyboardPhysicalState& state,
    const int scancode) {
    state.scancodes[static_cast<std::size_t>(scancode)] = true;
}

whacker::app::MenuIntent menu_intent(
    const bool up = false,
    const bool down = false,
    const bool left = false,
    const bool right = false,
    const bool confirm = false,
    const bool back = false) {
    return whacker::app::MenuIntent {
        .up = up,
        .down = down,
        .left = left,
        .right = right,
        .confirm = confirm,
        .back = back,
    };
}

void test_keyboard_edges_and_menu_actions() {
    whacker::app::KeyboardPhysicalState previous {};
    whacker::app::KeyboardPhysicalState current {};
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeW);
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeReturn);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_keyboard_action_frame(previous, current);

    TEST_CHECK(whacker::app::input_held(frame, whacker::app::InputAction::MenuUp));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::MenuUp));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::Confirm));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(frame.p1_move_y == -1.0f);
    TEST_CHECK(frame.p2_move_y == 0.0f);
}

void test_menu_intent_derives_pressed_and_held_edges() {
    whacker::app::KeyboardPhysicalState previous {};
    whacker::app::KeyboardPhysicalState current {};
    set_keyboard_scancode(previous, whacker::app::kKeyboardScancodeLeft);
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeLeft);
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeDown);
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeReturn);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_keyboard_action_frame(previous, current);
    const whacker::app::MenuInputIntent intent = whacker::app::derive_menu_input_intent(frame);

    TEST_CHECK(!intent.pressed.left);
    TEST_CHECK(intent.held.left);
    TEST_CHECK(intent.pressed.down);
    TEST_CHECK(intent.held.down);
    TEST_CHECK(intent.pressed.confirm);
    TEST_CHECK(!intent.pressed.back);
}

void test_keyboard_scancode_rebinding_drives_player_axis() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
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

    whacker::app::InputPhysicalState current {};
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeUp);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);

    TEST_CHECK(frame.p1_move_y == -1.0f);
    TEST_CHECK(frame.p2_move_y == -1.0f);
}

void test_held_keys_are_not_repeated_presses() {
    whacker::app::KeyboardPhysicalState previous {};
    set_keyboard_scancode(previous, whacker::app::kKeyboardScancodeDown);
    whacker::app::KeyboardPhysicalState current {};
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeDown);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_keyboard_action_frame(previous, current);

    TEST_CHECK(whacker::app::input_held(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(!whacker::app::input_released(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(frame.p2_move_y == 1.0f);
}

void test_released_key_reports_release_edge() {
    whacker::app::KeyboardPhysicalState previous {};
    set_keyboard_scancode(previous, whacker::app::kKeyboardScancodeEscape);
    whacker::app::KeyboardPhysicalState current {};

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_keyboard_action_frame(previous, current);

    TEST_CHECK(!whacker::app::input_held(frame, whacker::app::InputAction::Pause));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::Pause));
    TEST_CHECK(whacker::app::input_released(frame, whacker::app::InputAction::Pause));
}

void test_opposed_axis_cancels_to_zero() {
    whacker::app::KeyboardPhysicalState current {};
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeW);
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeS);
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeUp);
    set_keyboard_scancode(current, whacker::app::kKeyboardScancodeDown);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_keyboard_action_frame({}, current);

    TEST_CHECK(frame.p1_move_y == 0.0f);
    TEST_CHECK(frame.p2_move_y == 0.0f);
}

void test_main_menu_confirm_maps_every_visible_row() {
    whacker::app::MainMenuState menu_state {};

    menu_state.selected_row = whacker::app::MainMenuRowStory;
    TEST_CHECK(
        whacker::app::apply_main_menu_action(menu_state, menu_intent(false, false, false, false, true)) ==
        whacker::app::MainMenuActionResult::Story);

    menu_state.selected_row = whacker::app::MainMenuRowQuick;
    TEST_CHECK(
        whacker::app::apply_main_menu_action(menu_state, menu_intent(false, false, false, false, true)) ==
        whacker::app::MainMenuActionResult::Quick);

    menu_state.selected_row = whacker::app::MainMenuRowOptions;
    TEST_CHECK(
        whacker::app::apply_main_menu_action(menu_state, menu_intent(false, false, false, false, true)) ==
        whacker::app::MainMenuActionResult::Options);

    menu_state.selected_row = whacker::app::MainMenuRowQuit;
    TEST_CHECK(
        whacker::app::apply_main_menu_action(menu_state, menu_intent(false, false, false, false, true)) ==
        whacker::app::MainMenuActionResult::Quit);
}

void test_controller_menu_button_edges_share_action_state() {
    whacker::app::InputPhysicalState previous {};
    whacker::app::InputPhysicalState current {};
    set_controller_button(current, 0, whacker::app::ControllerButton::A);
    set_controller_button(current, 0, whacker::app::ControllerButton::Start);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame(previous, current);

    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::Confirm));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::Pause));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::Back));
}

void test_controller_axis_menu_edges_use_deadzone() {
    whacker::app::InputPhysicalState previous {};
    whacker::app::InputPhysicalState current {};
    set_controller_axis(previous, 0, whacker::app::ControllerAxis::LeftY, 0.20f);
    set_controller_axis(current, 0, whacker::app::ControllerAxis::LeftY, 0.70f);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame(previous, current);

    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::MenuDown));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::MenuUp));
}

void test_controller_axis_direction_binding_drives_menu_action() {
    whacker::app::ActionInputBindings bindings {};
    whacker::app::add_action_input_binding(
        bindings,
        whacker::app::action_binding_target(whacker::app::InputAction::MenuRight),
        whacker::app::controller_axis_direction_source(
            2,
            whacker::app::ControllerAxis::RightX,
            whacker::app::AxisDirection::Positive));

    whacker::app::InputPhysicalState current {};
    set_controller_axis(current, 2, whacker::app::ControllerAxis::RightX, 0.70f);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);

    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::MenuRight));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::MenuLeft));
}

void test_controller_defaults_drive_p1_and_p2_axes() {
    whacker::app::InputPhysicalState current {};
    set_controller_axis(current, 0, whacker::app::ControllerAxis::LeftY, -0.50f);
    set_controller_axis(current, 1, whacker::app::ControllerAxis::LeftY, 0.75f);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current);

    TEST_CHECK(frame.p1_move_y == -0.50f);
    TEST_CHECK(frame.p2_move_y == 0.75f);
}

void test_controller_player_rebinding_changes_slots_and_axis() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::bind_player_controller(bindings, whacker::app::InputSlot::P1, 1);
    whacker::app::bind_player_move_axis(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::ControllerAxis::RightY,
        true);

    whacker::app::InputPhysicalState current {};
    set_controller_axis(current, 0, whacker::app::ControllerAxis::LeftY, 1.0f);
    set_controller_axis(current, 1, whacker::app::ControllerAxis::RightY, 0.60f);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);

    TEST_CHECK(frame.p1_move_y == -0.60f);
    TEST_CHECK(frame.p2_move_y == 0.0f);
}

void test_controller_button_rebinding_contributes_to_axis() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::bind_player_move_buttons(
        bindings,
        whacker::app::InputSlot::P1,
        whacker::app::ControllerButton::LeftShoulder,
        whacker::app::ControllerButton::RightShoulder);

    whacker::app::InputPhysicalState current {};
    set_controller_button(current, 0, whacker::app::ControllerButton::RightShoulder);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);

    TEST_CHECK(frame.p1_move_y == 1.0f);
}

void test_controller_menu_rebinding_changes_buttons_and_axes() {
    whacker::app::ActionInputBindings bindings = whacker::app::default_action_input_bindings();
    whacker::app::bind_menu_controller(bindings, 1);
    whacker::app::bind_menu_axes(
        bindings,
        whacker::app::ControllerAxis::RightX,
        whacker::app::ControllerAxis::RightY);
    whacker::app::bind_menu_direction_buttons(
        bindings,
        whacker::app::ControllerButton::Y,
        whacker::app::ControllerButton::A,
        whacker::app::ControllerButton::X,
        whacker::app::ControllerButton::B);

    whacker::app::InputPhysicalState current {};
    set_controller_axis(current, 0, whacker::app::ControllerAxis::LeftY, 1.0f);
    set_controller_axis(current, 1, whacker::app::ControllerAxis::RightX, -0.80f);
    set_controller_button(current, 1, whacker::app::ControllerButton::Y);

    const whacker::app::ActionInputFrame frame =
        whacker::app::derive_action_input_frame({}, current, bindings);

    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::MenuUp));
    TEST_CHECK(whacker::app::input_pressed(frame, whacker::app::InputAction::MenuLeft));
    TEST_CHECK(!whacker::app::input_pressed(frame, whacker::app::InputAction::MenuDown));
}

whacker::app::MatchExitPolicy quick_exit_policy() {
    whacker::app::MatchExitPolicy policy {};
    policy.has_exit_option = true;
    policy.can_exit_now = true;
    policy.requires_confirmation = false;
    policy.exit_label = "EXIT MATCH";
    policy.action = whacker::app::MatchExitAction::ExitQuickToSetup;
    return policy;
}

void test_pause_menu_resume_exit_and_quit_results() {
    whacker::app::PauseMenuState pause {};
    const whacker::app::MatchExitPolicy policy = quick_exit_policy();

    pause.selected_row = whacker::app::PauseMenuRowResume;
    TEST_CHECK(
        whacker::app::apply_pause_menu_action(pause, policy, { .menu = menu_intent(false, false, false, false, true) }) ==
        whacker::app::PauseMenuActionResult::Resume);

    pause.selected_row = whacker::app::PauseMenuRowExitMatch;
    TEST_CHECK(
        whacker::app::apply_pause_menu_action(pause, policy, { .menu = menu_intent(false, false, false, false, true) }) ==
        whacker::app::PauseMenuActionResult::ExitMatch);

    pause.selected_row = whacker::app::PauseMenuRowQuitToMainMenu;
    TEST_CHECK(
        whacker::app::apply_pause_menu_action(pause, policy, { .menu = menu_intent(false, false, false, false, true) }) ==
        whacker::app::PauseMenuActionResult::QuitToMainMenu);
}

void test_pause_menu_confirmation_flow() {
    whacker::app::PauseMenuState pause {};
    whacker::app::MatchExitPolicy policy = quick_exit_policy();
    policy.requires_confirmation = true;
    pause.selected_row = whacker::app::PauseMenuRowExitMatch;

    TEST_CHECK(
        whacker::app::apply_pause_menu_action(pause, policy, { .menu = menu_intent(false, false, false, false, true) }) ==
        whacker::app::PauseMenuActionResult::None);
    TEST_CHECK(pause.confirm_forfeit);
    TEST_CHECK(pause.confirm_selected == 0);

    TEST_CHECK(
        whacker::app::apply_pause_menu_action(pause, policy, { .menu = menu_intent(false, false, false, true) }) ==
        whacker::app::PauseMenuActionResult::None);
    TEST_CHECK(pause.confirm_selected == 1);

    TEST_CHECK(
        whacker::app::apply_pause_menu_action(pause, policy, { .menu = menu_intent(false, false, false, false, true) }) ==
        whacker::app::PauseMenuActionResult::ExitMatch);
    TEST_CHECK(!pause.confirm_forfeit);
}

void test_pause_menu_back_resumes_or_cancels_confirmation() {
    whacker::app::PauseMenuState pause {};
    whacker::app::MatchExitPolicy policy = quick_exit_policy();
    policy.requires_confirmation = true;
    pause.confirm_forfeit = true;
    pause.confirm_selected = 1;

    TEST_CHECK(
        whacker::app::apply_pause_menu_action(pause, policy, { .menu = menu_intent(false, false, false, false, false, true) }) ==
        whacker::app::PauseMenuActionResult::None);
    TEST_CHECK(!pause.confirm_forfeit);
    TEST_CHECK(pause.confirm_selected == 0);

    pause.selected_row = whacker::app::PauseMenuRowExitMatch;
    TEST_CHECK(
        whacker::app::apply_pause_menu_action(pause, policy, { .pause = true }) ==
        whacker::app::PauseMenuActionResult::Resume);
    TEST_CHECK(pause.selected_row == whacker::app::PauseMenuRowResume);
}

void test_options_menu_audio_and_binding_flow() {
    whacker::app::OptionsMenuState options {};
    whacker::app::AudioSettings audio {};

    options.selected_row = whacker::app::OptionsMenuRowMasterVolume;
    audio.master_volume = 80;
    TEST_CHECK(
        whacker::app::apply_options_menu_action(options, audio, menu_intent(false, false, true)) ==
        whacker::app::OptionsMenuActionResult::AudioChanged);
    TEST_CHECK(audio.master_volume == 75);

    options.selected_row = whacker::app::OptionsMenuRowMute;
    TEST_CHECK(
        whacker::app::apply_options_menu_action(options, audio, menu_intent(false, false, false, false, true)) ==
        whacker::app::OptionsMenuActionResult::AudioChanged);
    TEST_CHECK(audio.mute);

    options.selected_row = whacker::app::OptionsMenuRowP1Up;
    TEST_CHECK(
        whacker::app::apply_options_menu_action(options, audio, menu_intent(false, false, false, false, true)) ==
        whacker::app::OptionsMenuActionResult::BindingCaptureStarted);
    TEST_CHECK(options.waiting_for_key);
    TEST_CHECK(
        whacker::app::apply_options_menu_action(options, audio, menu_intent(false, false, false, true)) ==
        whacker::app::OptionsMenuActionResult::BindingChanged);
    TEST_CHECK(options.waiting_for_key);
    TEST_CHECK(
        whacker::app::apply_options_menu_action(options, audio, menu_intent(false, false, false, false, false, true)) ==
        whacker::app::OptionsMenuActionResult::None);
    TEST_CHECK(!options.waiting_for_key);
}

void test_quick_menu_style_rows_cycle_defaults_and_confirm_tunes() {
    whacker::app::MenuState menu {};
    whacker::app::MatchOptions options {};

    menu.selected_row = whacker::app::MenuRowP1Tuning;
    const whacker::app::QuickMenuActionResult p1_style_result =
        whacker::app::apply_quick_menu_action(menu, options, menu_intent(false, false, false, true));
    TEST_CHECK(!p1_style_result.row_changed);
    TEST_CHECK(p1_style_result.options_changed);
    TEST_CHECK(!p1_style_result.start_requested);
    TEST_CHECK(!p1_style_result.tune_p1_requested);
    TEST_CHECK(options.left_ai_style == whacker::app::AiStyle::Power);
    TEST_CHECK(options.left_paddle_skills.power == whacker::app::ai_style_profile(whacker::app::AiStyle::Power).seed_skills.power);

    const whacker::app::QuickMenuActionResult tune_p1_result =
        whacker::app::apply_quick_menu_action(menu, options, menu_intent(false, false, false, false, true));
    TEST_CHECK(!tune_p1_result.options_changed);
    TEST_CHECK(tune_p1_result.tune_p1_requested);

    menu.selected_row = whacker::app::MenuRowP2Tuning;
    const whacker::app::QuickMenuActionResult p2_style_result =
        whacker::app::apply_quick_menu_action(menu, options, menu_intent(false, false, true));
    TEST_CHECK(!p2_style_result.row_changed);
    TEST_CHECK(p2_style_result.options_changed);
    TEST_CHECK(options.right_ai_style == whacker::app::AiStyle::Balanced);
    TEST_CHECK(options.right_paddle_skills.edge == whacker::app::ai_style_profile(whacker::app::AiStyle::Balanced).seed_skills.edge);

    menu.selected_row = whacker::app::MenuRowP1;
    const whacker::app::QuickMenuActionResult row_result =
        whacker::app::apply_quick_menu_action(menu, options, menu_intent(false, true));
    TEST_CHECK(row_result.row_changed);
    TEST_CHECK(!row_result.options_changed);
    TEST_CHECK(menu.selected_row == whacker::app::MenuRowP2);

    menu.selected_row = whacker::app::MenuRowStart;
    const whacker::app::QuickMenuActionResult start_result =
        whacker::app::apply_quick_menu_action(menu, options, menu_intent(false, false, false, false, true));
    TEST_CHECK(start_result.start_requested);
    TEST_CHECK(!start_result.options_changed);

    const whacker::app::QuickMenuActionResult back_result =
        whacker::app::apply_quick_menu_action(menu, options, menu_intent(false, true, false, false, false, true));
    TEST_CHECK(back_result.back_requested);
    TEST_CHECK(!back_result.row_changed);
}

void test_story_menu_continue_new_career_and_back_flow() {
    whacker::app::StoryMenuState story {};

    story.selected_row = whacker::app::StoryMenuRowContinue;
    TEST_CHECK(
        whacker::app::apply_story_menu_action(story, false, menu_intent(false, false, false, false, true)) ==
        whacker::app::StoryMenuActionResult::None);

    TEST_CHECK(
        whacker::app::apply_story_menu_action(story, true, menu_intent(false, false, false, false, true)) ==
        whacker::app::StoryMenuActionResult::Continue);

    story.selected_row = whacker::app::StoryMenuRowNewCareer;
    TEST_CHECK(
        whacker::app::apply_story_menu_action(story, false, menu_intent(false, false, false, false, true)) ==
        whacker::app::StoryMenuActionResult::NewCareer);

    story.selected_row = whacker::app::StoryMenuRowNewCareer;
    TEST_CHECK(
        whacker::app::apply_story_menu_action(story, true, menu_intent(false, false, false, false, true)) ==
        whacker::app::StoryMenuActionResult::None);
    TEST_CHECK(story.confirm_overwrite);
    TEST_CHECK(story.confirm_selected == 0);
    TEST_CHECK(
        whacker::app::apply_story_menu_action(story, true, menu_intent(false, false, false, true)) ==
        whacker::app::StoryMenuActionResult::None);
    TEST_CHECK(story.confirm_selected == 1);
    TEST_CHECK(
        whacker::app::apply_story_menu_action(story, true, menu_intent(false, false, false, false, true)) ==
        whacker::app::StoryMenuActionResult::NewCareer);
    TEST_CHECK(!story.confirm_overwrite);

    story.selected_row = whacker::app::StoryMenuRowContinue;
    TEST_CHECK(
        whacker::app::apply_story_menu_action(story, false, menu_intent(false, false, false, false, false, true)) ==
        whacker::app::StoryMenuActionResult::Back);
    TEST_CHECK(story.selected_row == whacker::app::StoryMenuRowBack);
}

}  // namespace

int main() {
    test_keyboard_edges_and_menu_actions();
    test_menu_intent_derives_pressed_and_held_edges();
    test_keyboard_scancode_rebinding_drives_player_axis();
    test_held_keys_are_not_repeated_presses();
    test_released_key_reports_release_edge();
    test_opposed_axis_cancels_to_zero();
    test_main_menu_confirm_maps_every_visible_row();
    test_controller_menu_button_edges_share_action_state();
    test_controller_axis_menu_edges_use_deadzone();
    test_controller_axis_direction_binding_drives_menu_action();
    test_controller_defaults_drive_p1_and_p2_axes();
    test_controller_player_rebinding_changes_slots_and_axis();
    test_controller_button_rebinding_contributes_to_axis();
    test_controller_menu_rebinding_changes_buttons_and_axes();
    test_pause_menu_resume_exit_and_quit_results();
    test_pause_menu_confirmation_flow();
    test_pause_menu_back_resumes_or_cancels_confirmation();
    test_options_menu_audio_and_binding_flow();
    test_quick_menu_style_rows_cycle_defaults_and_confirm_tunes();
    test_story_menu_continue_new_career_and_back_flow();
    return 0;
}
