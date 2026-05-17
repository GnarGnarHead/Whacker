#include "test_assert.hpp"
#include <random>
#include <string>

#include <GLFW/glfw3.h>

#include "runtime_escape.hpp"
#include "runtime_pause.hpp"
#include "runtime_story_scene.hpp"
#include "story_text.hpp"

namespace {

struct StubState {
    bool stub_confirm_press = false;
    bool stub_menu_up = false;
    bool stub_menu_down = false;
    bool stub_key_left = false;
    bool stub_key_right = false;
    bool clear_last_pressed_called = false;
    int window_close_calls = 0;
    bool execute_pause_exit_called = false;
    bool quit_to_main_called = false;
    bool clear_story_scene_called = false;
    bool reveal_scene_called = false;
    bool advance_scene_result = false;
    bool start_story_match_called = false;
    whacker::app::StoryMatchKind start_story_match_kind = whacker::app::StoryMatchKind::None;
    bool copy_onboarding_called = false;
    bool save_called = false;

    void reset() {
        *this = StubState {};
    }
};

StubState g_stub_state {};
bool& g_stub_confirm_press = g_stub_state.stub_confirm_press;
bool& g_stub_menu_up = g_stub_state.stub_menu_up;
bool& g_stub_menu_down = g_stub_state.stub_menu_down;
bool& g_stub_key_left = g_stub_state.stub_key_left;
bool& g_stub_key_right = g_stub_state.stub_key_right;
bool& g_clear_last_pressed_called = g_stub_state.clear_last_pressed_called;
int& g_window_close_calls = g_stub_state.window_close_calls;
bool& g_execute_pause_exit_called = g_stub_state.execute_pause_exit_called;
bool& g_quit_to_main_called = g_stub_state.quit_to_main_called;
bool& g_clear_story_scene_called = g_stub_state.clear_story_scene_called;
bool& g_reveal_scene_called = g_stub_state.reveal_scene_called;
bool& g_advance_scene_result = g_stub_state.advance_scene_result;
bool& g_start_story_match_called = g_stub_state.start_story_match_called;
whacker::app::StoryMatchKind& g_start_story_match_kind = g_stub_state.start_story_match_kind;
bool& g_copy_onboarding_called = g_stub_state.copy_onboarding_called;
bool& g_save_called = g_stub_state.save_called;

void reset_stubs() {
    g_stub_state.reset();
}

bool fake_save_career(const whacker::app::StoryCareerData&, std::string*) {
    g_save_called = true;
    return true;
}

struct EscapeFixture {
    whacker::app::AppState app_state = whacker::app::AppState::MainMenu;
    whacker::app::AppState pause_return_state = whacker::app::AppState::Playing;
    whacker::app::StoryMenuState story_menu {};
    whacker::app::OptionsMenuState options_menu {};
    whacker::app::PauseMenuState pause_menu {};
    whacker::app::StorySceneState scene {};
    whacker::app::PaddleTuningState paddle_tuning {};
    whacker::app::StoryRuntimeState runtime {};

    bool run() {
        return whacker::app::handle_runtime_escape_key(
            nullptr,
            app_state,
            pause_return_state,
            story_menu,
            options_menu,
            pause_menu,
            scene,
            paddle_tuning,
            runtime);
    }
};

struct PauseFixture {
    whacker::app::KeyEdgeState edge {};
    whacker::app::ControlBindings controls {};
    whacker::app::MatchExitPolicy policy {};
    whacker::app::PauseMenuState pause_menu {};
    whacker::app::AppState app_state = whacker::app::AppState::Paused;
    whacker::app::AppState pause_return_state = whacker::app::AppState::Playing;
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    whacker::app::StoryIntroState intro {};
    whacker::app::StorySceneState scene {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::sim::Simulation simulation {};
    int story_official_games_to_win = 3;

    whacker::app::PauseInputFeedback run() {
        return whacker::app::handle_runtime_pause_input(
            nullptr,
            edge,
            controls,
            policy,
            pause_menu,
            app_state,
            pause_return_state,
            runtime,
            hub,
            intro,
            scene,
            authored_transition_request,
            match_flow,
            simulation,
            story_official_games_to_win,
            nullptr,
            fake_save_career);
    }
};

struct StorySceneConfirmFixture {
    whacker::app::StorySceneState scene {};
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    whacker::app::MatchOptions options {};
    whacker::app::MatchFlowState match_flow {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng {0xC0FFEEULL};
    whacker::app::AppState app_state = whacker::app::AppState::StoryScene;
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};

    void run() {
        whacker::app::handle_story_scene_confirm(
            scene,
            runtime,
            hub,
            options,
            match_flow,
            simulation,
            rng,
            app_state,
            authored_transition_request,
            fake_save_career);
    }
};

void test_escape_from_playing_enters_pause() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::Playing;
    fixture.pause_return_state = whacker::app::AppState::MainMenu;
    fixture.pause_menu.confirm_forfeit = true;
    fixture.pause_menu.confirm_selected = 1;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(played_confirm);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::Paused);
    TEST_CHECK(fixture.pause_return_state == whacker::app::AppState::Playing);
    TEST_CHECK(fixture.pause_menu.selected_row == whacker::app::PauseMenuRowResume);
    TEST_CHECK(!fixture.pause_menu.confirm_forfeit);
    TEST_CHECK(fixture.pause_menu.confirm_selected == 0);
}

void test_escape_from_story_scene_returns_to_story_menu() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::StoryScene;
    fixture.scene.id = whacker::app::StorySceneId::OnboardingClubIntro;
    fixture.runtime.onboarding_scene_pending = true;
    fixture.runtime.post_forfeit_scene_pending = true;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(played_confirm);
    TEST_CHECK(g_clear_story_scene_called);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryMenu);
    TEST_CHECK(!fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(!fixture.runtime.post_forfeit_scene_pending);
}

void test_escape_in_options_waiting_for_key_clears_waiting_and_consumes_key() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::OptionsMenu;
    fixture.options_menu.waiting_for_key = true;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(played_confirm);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::OptionsMenu);
    TEST_CHECK(!fixture.options_menu.waiting_for_key);
    TEST_CHECK(g_clear_last_pressed_called);
    TEST_CHECK(g_window_close_calls == 0);
}

void test_escape_in_options_without_waiting_routes_to_main_menu() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::OptionsMenu;
    fixture.options_menu.waiting_for_key = false;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(played_confirm);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(!fixture.options_menu.waiting_for_key);
    TEST_CHECK(!g_clear_last_pressed_called);
    TEST_CHECK(g_window_close_calls == 0);
}

void test_escape_from_main_menu_requests_window_close_without_confirm_sound() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::MainMenu;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(!played_confirm);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(g_window_close_calls == 1);
    TEST_CHECK(!g_clear_last_pressed_called);
}

void test_escape_in_story_menu_modal_clears_overwrite_dialog() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::StoryMenu;
    fixture.story_menu.confirm_overwrite = true;
    fixture.story_menu.confirm_selected = 1;
    fixture.runtime.onboarding_scene_pending = true;
    fixture.runtime.post_forfeit_scene_pending = true;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(played_confirm);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryMenu);
    TEST_CHECK(fixture.story_menu.confirm_overwrite == false);
    TEST_CHECK(fixture.story_menu.confirm_selected == 0);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(fixture.runtime.post_forfeit_scene_pending);
    TEST_CHECK(fixture.pause_menu.selected_row == whacker::app::PauseMenuRowResume);
    TEST_CHECK(fixture.pause_menu.confirm_forfeit == false);
    TEST_CHECK(fixture.pause_menu.confirm_selected == 0);
}

void test_escape_from_story_menu_without_modal_goes_to_main_menu() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::StoryMenu;
    fixture.story_menu.confirm_overwrite = false;
    fixture.story_menu.confirm_selected = 0;
    fixture.runtime.onboarding_scene_pending = true;
    fixture.runtime.post_forfeit_scene_pending = true;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(played_confirm);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(fixture.story_menu.confirm_overwrite == false);
    TEST_CHECK(fixture.story_menu.confirm_selected == 0);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(fixture.runtime.post_forfeit_scene_pending);
    TEST_CHECK(fixture.pause_menu.selected_row == whacker::app::PauseMenuRowResume);
    TEST_CHECK(fixture.pause_menu.confirm_forfeit == false);
    TEST_CHECK(fixture.pause_menu.confirm_selected == 0);
}

void test_escape_from_paused_with_confirm_forfeit_clears_prompt() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::Paused;
    fixture.pause_return_state = whacker::app::AppState::StoryHub;
    fixture.pause_menu.selected_row = whacker::app::PauseMenuRowExitMatch;
    fixture.pause_menu.confirm_forfeit = true;
    fixture.pause_menu.confirm_selected = 1;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(played_confirm);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::Paused);
    TEST_CHECK(fixture.pause_menu.selected_row == whacker::app::PauseMenuRowExitMatch);
    TEST_CHECK(fixture.pause_menu.confirm_forfeit == false);
    TEST_CHECK(fixture.pause_menu.confirm_selected == 0);
}

void test_escape_from_paused_without_confirm_forfeit_resumes_previous_state() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::Paused;
    fixture.pause_return_state = whacker::app::AppState::StoryIntro;
    fixture.pause_menu.selected_row = whacker::app::PauseMenuRowQuitToMainMenu;
    fixture.pause_menu.confirm_forfeit = false;
    fixture.pause_menu.confirm_selected = 1;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(played_confirm);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(fixture.pause_menu.selected_row == whacker::app::PauseMenuRowResume);
    TEST_CHECK(fixture.pause_menu.confirm_forfeit == false);
    TEST_CHECK(fixture.pause_menu.confirm_selected == 1);
}

void test_escape_from_paddle_tuning_returns_to_stored_state() {
    reset_stubs();
    EscapeFixture fixture {};
    fixture.app_state = whacker::app::AppState::PaddleTuning;
    fixture.paddle_tuning.active = true;
    fixture.paddle_tuning.return_state = whacker::app::AppState::StoryHub;

    const bool played_confirm = fixture.run();
    static_cast<void>(played_confirm);

    TEST_CHECK(played_confirm);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(!fixture.paddle_tuning.active);
}

void test_pause_resume_confirm_returns_to_previous_state() {
    reset_stubs();
    PauseFixture fixture {};
    fixture.pause_menu.selected_row = whacker::app::PauseMenuRowResume;
    fixture.pause_return_state = whacker::app::AppState::StoryIntro;
    g_stub_confirm_press = true;

    const whacker::app::PauseInputFeedback feedback = fixture.run();
    static_cast<void>(feedback);

    TEST_CHECK(feedback.play_menu_confirm);
    TEST_CHECK(!feedback.play_menu_move);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
}

void test_pause_confirm_forfeit_toggle_marks_move_feedback() {
    reset_stubs();
    PauseFixture fixture {};
    fixture.policy.has_exit_option = true;
    fixture.policy.can_exit_now = true;
    fixture.policy.requires_confirmation = true;
    fixture.pause_menu.confirm_forfeit = true;
    fixture.pause_menu.confirm_selected = 0;
    fixture.pause_return_state = whacker::app::AppState::StoryIntro;
    g_stub_menu_up = true;

    const whacker::app::PauseInputFeedback feedback = fixture.run();
    static_cast<void>(feedback);

    TEST_CHECK(!feedback.play_menu_confirm);
    TEST_CHECK(feedback.play_menu_move);
    TEST_CHECK(fixture.pause_menu.confirm_forfeit);
    TEST_CHECK(fixture.pause_menu.confirm_selected == 1);
    TEST_CHECK(!g_execute_pause_exit_called);
}

void test_pause_confirm_forfeit_cancel_confirm_clears_prompt() {
    reset_stubs();
    PauseFixture fixture {};
    fixture.policy.has_exit_option = true;
    fixture.policy.can_exit_now = true;
    fixture.policy.requires_confirmation = true;
    fixture.pause_menu.confirm_forfeit = true;
    fixture.pause_menu.confirm_selected = 0;
    fixture.pause_return_state = whacker::app::AppState::StoryIntro;
    g_stub_confirm_press = true;

    const whacker::app::PauseInputFeedback feedback = fixture.run();
    static_cast<void>(feedback);

    TEST_CHECK(feedback.play_menu_confirm);
    TEST_CHECK(!feedback.play_menu_move);
    TEST_CHECK(!fixture.pause_menu.confirm_forfeit);
    TEST_CHECK(fixture.pause_menu.confirm_selected == 0);
    TEST_CHECK(!g_execute_pause_exit_called);
}

void test_pause_confirm_forfeit_accept_confirm_executes_exit() {
    reset_stubs();
    PauseFixture fixture {};
    fixture.policy.has_exit_option = true;
    fixture.policy.can_exit_now = true;
    fixture.policy.requires_confirmation = true;
    fixture.policy.action = whacker::app::MatchExitAction::ExitStoryMatch;
    fixture.pause_menu.confirm_forfeit = true;
    fixture.pause_menu.confirm_selected = 1;
    fixture.pause_return_state = whacker::app::AppState::StoryIntro;
    g_stub_confirm_press = true;

    const whacker::app::PauseInputFeedback feedback = fixture.run();
    static_cast<void>(feedback);

    TEST_CHECK(feedback.play_menu_confirm);
    TEST_CHECK(!feedback.play_menu_move);
    TEST_CHECK(fixture.pause_menu.confirm_forfeit);
    TEST_CHECK(fixture.pause_menu.confirm_selected == 1);
    TEST_CHECK(g_execute_pause_exit_called);
}

void test_pause_confirm_prompt_auto_clears_when_policy_no_longer_requires_confirmation() {
    reset_stubs();
    PauseFixture fixture {};
    fixture.policy.has_exit_option = true;
    fixture.policy.can_exit_now = true;
    fixture.policy.requires_confirmation = false;
    fixture.pause_menu.confirm_forfeit = true;
    fixture.pause_menu.confirm_selected = 1;
    fixture.pause_return_state = whacker::app::AppState::StoryIntro;

    const whacker::app::PauseInputFeedback feedback = fixture.run();
    static_cast<void>(feedback);

    TEST_CHECK(!feedback.play_menu_confirm);
    TEST_CHECK(!feedback.play_menu_move);
    TEST_CHECK(!fixture.pause_menu.confirm_forfeit);
    TEST_CHECK(fixture.pause_menu.confirm_selected == 0);
    TEST_CHECK(!g_execute_pause_exit_called);
}

void test_pause_exit_row_when_cannot_exit_now_bounces_to_resume() {
    reset_stubs();
    PauseFixture fixture {};
    fixture.policy.has_exit_option = true;
    fixture.policy.can_exit_now = false;
    fixture.policy.requires_confirmation = true;
    fixture.policy.action = whacker::app::MatchExitAction::ExitStoryMatch;
    fixture.pause_menu.selected_row = whacker::app::PauseMenuRowExitMatch;
    fixture.pause_return_state = whacker::app::AppState::Playing;
    g_stub_confirm_press = true;

    const whacker::app::PauseInputFeedback feedback = fixture.run();
    static_cast<void>(feedback);

    TEST_CHECK(feedback.play_menu_confirm);
    TEST_CHECK(!feedback.play_menu_move);
    TEST_CHECK(fixture.pause_menu.selected_row == whacker::app::PauseMenuRowResume);
    TEST_CHECK(!fixture.pause_menu.confirm_forfeit);
    TEST_CHECK(!g_execute_pause_exit_called);
    TEST_CHECK(!g_quit_to_main_called);
}

void test_pause_without_exit_option_clamps_row_and_confirm_routes_to_quit() {
    reset_stubs();
    PauseFixture fixture {};
    fixture.policy.has_exit_option = false;
    fixture.policy.can_exit_now = false;
    fixture.policy.requires_confirmation = false;
    fixture.pause_menu.selected_row = whacker::app::PauseMenuRowQuitToMainMenu;
    fixture.pause_return_state = whacker::app::AppState::Playing;
    g_stub_confirm_press = true;

    const whacker::app::PauseInputFeedback feedback = fixture.run();
    static_cast<void>(feedback);

    TEST_CHECK(feedback.play_menu_confirm);
    TEST_CHECK(!feedback.play_menu_move);
    TEST_CHECK(fixture.pause_menu.selected_row == whacker::app::PauseMenuRowExitMatch);
    TEST_CHECK(g_quit_to_main_called);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
}

void test_pause_exit_match_without_confirmation_executes() {
    reset_stubs();
    PauseFixture fixture {};
    fixture.policy.has_exit_option = true;
    fixture.policy.can_exit_now = true;
    fixture.policy.requires_confirmation = false;
    fixture.policy.action = whacker::app::MatchExitAction::ExitStoryMatch;
    fixture.pause_menu.selected_row = whacker::app::PauseMenuRowExitMatch;
    fixture.pause_return_state = whacker::app::AppState::Playing;
    g_stub_confirm_press = true;

    const whacker::app::PauseInputFeedback feedback = fixture.run();
    static_cast<void>(feedback);

    TEST_CHECK(feedback.play_menu_confirm);
    TEST_CHECK(g_execute_pause_exit_called);
}

void test_pause_quit_row_calls_quit_to_main() {
    reset_stubs();
    PauseFixture fixture {};
    fixture.policy.has_exit_option = true;
    fixture.policy.can_exit_now = true;
    fixture.policy.requires_confirmation = true;
    fixture.policy.action = whacker::app::MatchExitAction::ExitStoryMatch;
    fixture.pause_menu.selected_row = whacker::app::PauseMenuRowQuitToMainMenu;
    fixture.pause_return_state = whacker::app::AppState::Playing;
    g_stub_confirm_press = true;

    const whacker::app::PauseInputFeedback feedback = fixture.run();
    static_cast<void>(feedback);

    TEST_CHECK(feedback.play_menu_confirm);
    TEST_CHECK(g_quit_to_main_called);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
}

void test_story_scene_dialogue_writing_reveals_line() {
    reset_stubs();
    StorySceneConfirmFixture fixture {};
    fixture.scene.dialogue_writing = true;
    fixture.run();

    TEST_CHECK(g_reveal_scene_called);
    TEST_CHECK(!g_start_story_match_called);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
}

void test_story_scene_early_arrival_starts_aya_match() {
    reset_stubs();
    g_advance_scene_result = true;
    StorySceneConfirmFixture fixture {};
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::EarlyArrivalScene;
    fixture.run();

    TEST_CHECK(g_start_story_match_called);
    TEST_CHECK(g_start_story_match_kind == whacker::app::StoryMatchKind::OnboardingAyaFriendly);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::AyaFriendlyMatch);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
}

void test_story_scene_club_intro_starts_entry_match() {
    reset_stubs();
    g_advance_scene_result = true;
    StorySceneConfirmFixture fixture {};
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::ClubIntroScene;
    fixture.run();

    TEST_CHECK(g_start_story_match_called);
    TEST_CHECK(g_start_story_match_kind == whacker::app::StoryMatchKind::OnboardingEntry);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::EntryBenchmarkMatch);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(!g_copy_onboarding_called);
    TEST_CHECK(!g_save_called);
}

void test_story_scene_entry_retry_starts_entry_match() {
    reset_stubs();
    g_advance_scene_result = true;
    StorySceneConfirmFixture fixture {};
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::EntryRetryScene;
    fixture.run();

    TEST_CHECK(g_start_story_match_called);
    TEST_CHECK(g_start_story_match_kind == whacker::app::StoryMatchKind::OnboardingEntry);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::EntryBenchmarkMatch);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(!g_copy_onboarding_called);
    TEST_CHECK(!g_save_called);
}

void test_story_scene_unknown_step_falls_back_to_story_hub() {
    reset_stubs();
    g_advance_scene_result = true;
    StorySceneConfirmFixture fixture {};
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::None;
    fixture.run();

    TEST_CHECK(!g_start_story_match_called);
    TEST_CHECK(!g_copy_onboarding_called);
    TEST_CHECK(!g_save_called);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
}

void test_story_scene_coach_brief_queues_at_home_scene_and_saves() {
    reset_stubs();
    g_advance_scene_result = true;
    StorySceneConfirmFixture fixture {};
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;
    fixture.run();

    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::AtHomeYoutubeScene);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(!fixture.runtime.career.joined_club);
    TEST_CHECK(g_copy_onboarding_called);
    TEST_CHECK(g_save_called);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
}

void test_story_scene_tix_midweek_yes_starts_lunch_match() {
    reset_stubs();
    g_advance_scene_result = true;
    StorySceneConfirmFixture fixture {};
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::TixMidweekScene;
    fixture.scene.has_binary_choice = true;
    fixture.scene.binary_choice_yes_selected = true;
    fixture.run();

    TEST_CHECK(g_start_story_match_called);
    TEST_CHECK(g_start_story_match_kind == whacker::app::StoryMatchKind::TixLunch);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::Complete);
    TEST_CHECK(fixture.runtime.career.tix_midweek_scene_seen);
    TEST_CHECK(fixture.runtime.career.tix_lunch_match_accepted);
    TEST_CHECK(!fixture.runtime.career.tix_lunch_match_declined);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(g_copy_onboarding_called);
    TEST_CHECK(g_save_called);
}

void test_story_scene_tix_midweek_no_declines_once_and_returns_hub() {
    reset_stubs();
    g_advance_scene_result = true;
    StorySceneConfirmFixture fixture {};
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::TixMidweekScene;
    fixture.scene.has_binary_choice = true;
    fixture.scene.binary_choice_yes_selected = false;
    fixture.run();

    TEST_CHECK(!g_start_story_match_called);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::Complete);
    TEST_CHECK(fixture.runtime.career.tix_midweek_scene_seen);
    TEST_CHECK(!fixture.runtime.career.tix_lunch_match_accepted);
    TEST_CHECK(fixture.runtime.career.tix_lunch_match_declined);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(g_copy_onboarding_called);
    TEST_CHECK(g_save_called);
}

}  // namespace

extern "C" void glfwSetWindowShouldClose(GLFWwindow* /*window*/, int /*value*/) {
    ++g_window_close_calls;
}

namespace whacker::app {

bool consume_key_press(GLFWwindow* /*window*/, int key, bool& previous_down) {
    bool pressed = false;
    if (key == GLFW_KEY_LEFT) {
        pressed = g_stub_key_left;
        g_stub_key_left = false;
    } else if (key == GLFW_KEY_RIGHT) {
        pressed = g_stub_key_right;
        g_stub_key_right = false;
    }
    previous_down = false;
    return pressed;
}

bool consume_confirm_press(GLFWwindow* /*window*/, KeyEdgeState& /*edge_state*/) {
    const bool pressed = g_stub_confirm_press;
    g_stub_confirm_press = false;
    return pressed;
}

bool consume_menu_up_press(GLFWwindow* /*window*/, KeyEdgeState& /*edge_state*/, const ControlBindings& /*controls*/) {
    const bool pressed = g_stub_menu_up;
    g_stub_menu_up = false;
    return pressed;
}

bool consume_menu_down_press(GLFWwindow* /*window*/, KeyEdgeState& /*edge_state*/, const ControlBindings& /*controls*/) {
    const bool pressed = g_stub_menu_down;
    g_stub_menu_down = false;
    return pressed;
}

void clear_last_pressed_key() {
    g_clear_last_pressed_called = true;
}

void execute_runtime_pause_exit(
    const MatchExitPolicy& /*policy*/,
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    StoryIntroState& /*story_intro_state*/,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    StorySceneState& /*story_scene_state*/,
    RuntimeAuthoredTransitionRequest& /*authored_transition_request*/,
    AppState& /*app_state*/,
    int /*story_official_games_to_win*/,
    StorySanitizeNameFn /*sanitize_name_fn*/,
    StorySaveCareerCallback /*save_career_fn*/) {
    g_execute_pause_exit_called = true;
}

void quit_runtime_to_main_menu(
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    StoryIntroState& /*story_intro_state*/,
    StorySceneState& /*story_scene_state*/,
    MatchFlowState& /*match_flow*/,
    PauseMenuState& /*pause_menu_state*/,
    AppState& /*pause_return_state*/,
    whacker::sim::Simulation& /*simulation*/,
    RuntimeAuthoredTransitionRequest& /*authored_transition_request*/,
    int /*story_official_games_to_win*/,
    StorySaveCareerCallback /*save_career_fn*/,
    AppState& app_state) {
    g_quit_to_main_called = true;
    app_state = AppState::MainMenu;
}

void clear_story_scene(StorySceneState& scene_state) {
    g_clear_story_scene_called = true;
    scene_state = StorySceneState {};
}

void reveal_story_scene_current_line(StorySceneState& scene_state) {
    g_reveal_scene_called = true;
    scene_state.dialogue_writing = false;
}

bool advance_story_scene(StorySceneState& /*scene_state*/) {
    return g_advance_scene_result;
}

void begin_story_onboarding_scene(
    StorySceneState& scene_state,
    const StoryRuntimeState& story_runtime) {
    scene_state = StorySceneState {};
    scene_state.id = story_runtime.post_forfeit_scene_pending
        ? StorySceneId::PostForfeitSupport
        : StorySceneId::OnboardingEarlyArrival;
    scene_state.line_count = 1;
    scene_state.dialogue_writing = false;
}

TransitionArmResult arm_authored_star_wipe_transition(
    RuntimeAuthoredTransitionRequest& request,
    const AppState from_state,
    const StorySceneState* from_story_scene,
    const AppState to_state,
    const StorySceneState* to_story_scene,
    const float /*duration_seconds*/) {
    request.armed = true;
    request.from_state = from_state;
    request.to_state = to_state;
    request.has_from_story_scene = from_story_scene != nullptr;
    request.has_to_story_scene = to_story_scene != nullptr;
    request.from_story_scene = from_story_scene != nullptr ? *from_story_scene : StorySceneState {};
    request.to_story_scene = to_story_scene != nullptr ? *to_story_scene : StorySceneState {};
    return TransitionArmResult {.armed = true, .error = TransitionArmError::None};
}

void start_story_match(
    StoryRuntimeState& story_runtime,
    StoryHubState& /*story_hub_state*/,
    MatchOptions& /*options*/,
    whacker::sim::Simulation& /*simulation*/,
    MatchFlowState& /*match_flow*/,
    std::mt19937_64& /*rng*/,
    const StoryMatchKind match_kind) {
    g_start_story_match_called = true;
    g_start_story_match_kind = match_kind;
    story_runtime.active_match = match_kind;
}

void copy_onboarding_runtime_to_career(StoryRuntimeState& story_runtime) {
    g_copy_onboarding_called = true;
    story_runtime.career.onboarding_step = story_runtime.onboarding_step;
}

bool story_graph_initialize_career_node(StoryCareerData& career) {
    if (career.progression_node_id.empty()) {
        career.progression_node_id = "stub-node";
        return true;
    }
    return false;
}

namespace story_text {

FeedbackLines onboarding_complete_feedback() {
    return FeedbackLines {.line_1 = "feedback-1", .line_2 = "feedback-2"};
}

}  // namespace story_text

}  // namespace whacker::app

int main() {
    test_escape_from_playing_enters_pause();
    test_escape_from_story_scene_returns_to_story_menu();
    test_escape_in_options_waiting_for_key_clears_waiting_and_consumes_key();
    test_escape_in_options_without_waiting_routes_to_main_menu();
    test_escape_from_main_menu_requests_window_close_without_confirm_sound();
    test_escape_in_story_menu_modal_clears_overwrite_dialog();
    test_escape_from_story_menu_without_modal_goes_to_main_menu();
    test_escape_from_paused_with_confirm_forfeit_clears_prompt();
    test_escape_from_paused_without_confirm_forfeit_resumes_previous_state();
    test_escape_from_paddle_tuning_returns_to_stored_state();
    test_pause_resume_confirm_returns_to_previous_state();
    test_pause_confirm_forfeit_toggle_marks_move_feedback();
    test_pause_confirm_forfeit_cancel_confirm_clears_prompt();
    test_pause_confirm_forfeit_accept_confirm_executes_exit();
    test_pause_confirm_prompt_auto_clears_when_policy_no_longer_requires_confirmation();
    test_pause_exit_row_when_cannot_exit_now_bounces_to_resume();
    test_pause_without_exit_option_clamps_row_and_confirm_routes_to_quit();
    test_pause_exit_match_without_confirmation_executes();
    test_pause_quit_row_calls_quit_to_main();
    test_story_scene_dialogue_writing_reveals_line();
    test_story_scene_early_arrival_starts_aya_match();
    test_story_scene_club_intro_starts_entry_match();
    test_story_scene_entry_retry_starts_entry_match();
    test_story_scene_unknown_step_falls_back_to_story_hub();
    test_story_scene_coach_brief_queues_at_home_scene_and_saves();
    test_story_scene_tix_midweek_yes_starts_lunch_match();
    test_story_scene_tix_midweek_no_declines_once_and_returns_hub();
    return 0;
}
