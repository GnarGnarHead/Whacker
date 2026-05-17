#include "test_assert.hpp"
#include <cmath>
#include <cstdint>
#include <random>
#include <string>

#include <GLFW/glfw3.h>

#include "runtime_input_phase.hpp"
#include "runtime_pause.hpp"
#include "runtime_transitions.hpp"
#include "story_flow.hpp"

namespace {

struct StubState {
    bool stub_escape = false;
    bool stub_menu_key = false;
    bool stub_dev_info_key = false;
    bool stub_dev_player_ai_key = false;
    bool stub_confirm_press = false;
    bool stub_menu_up = false;
    bool stub_menu_down = false;
    bool stub_key_left = false;
    bool stub_key_right = false;
    bool stub_key_left_held = false;
    bool stub_key_right_held = false;
    bool escape_played_confirm = false;
    int begin_story_scene_calls = 0;
    int story_scene_confirm_calls = 0;
    int audio_event_calls = 0;
    int audio_menu_move_calls = 0;
    int audio_menu_confirm_calls = 0;
    bool handle_escape_called = false;
    int main_menu_input_calls = 0;
    int story_hub_input_calls = 0;
    int options_menu_input_calls = 0;
    int quick_menu_input_calls = 0;
    int story_intro_input_calls = 0;
    int story_menu_input_calls = 0;
    int pause_input_calls = 0;
    int match_exit_policy_calls = 0;
    int save_menu_settings_calls = 0;
    int audio_set_settings_calls = 0;
    int story_save_exists_calls = 0;
    bool story_save_exists_result = false;
    bool stub_options_changed_bindings = false;
    bool stub_options_changed_audio_settings = false;
    bool stub_main_menu_change_row = false;
    bool stub_main_menu_change_state = false;
    bool stub_options_change_row = false;
    bool stub_options_toggle_waiting = false;
    bool stub_options_change_state = false;
    bool stub_quick_menu_change_row = false;
    bool stub_quick_menu_change_options = false;
    bool stub_quick_menu_change_state = false;
    whacker::app::AppState stub_quick_menu_next_state = whacker::app::AppState::MainMenu;
    bool stub_story_intro_change_phase = false;
    bool stub_story_intro_toggle_accept = false;
    bool stub_story_intro_change_state = false;
    bool stub_story_menu_move_selection = false;
    bool stub_story_menu_change_confirm_selected = false;
    bool stub_story_menu_change_state = false;
    bool stub_story_menu_toggle_confirm_overwrite = false;
    bool stub_story_hub_change_row = false;
    bool stub_story_hub_change_week = false;
    bool stub_story_hub_change_state = false;
    whacker::app::AppState stub_story_hub_next_state = whacker::app::AppState::MainMenu;
    bool stub_pause_feedback_move = false;
    bool stub_pause_feedback_confirm = false;
    int story_save_career_calls = 0;

    void reset() {
        *this = StubState {};
    }
};

StubState g_stub_state {};
bool& g_stub_escape = g_stub_state.stub_escape;
bool& g_stub_menu_key = g_stub_state.stub_menu_key;
bool& g_stub_dev_info_key = g_stub_state.stub_dev_info_key;
bool& g_stub_dev_player_ai_key = g_stub_state.stub_dev_player_ai_key;
bool& g_stub_confirm_press = g_stub_state.stub_confirm_press;
bool& g_stub_menu_up = g_stub_state.stub_menu_up;
bool& g_stub_menu_down = g_stub_state.stub_menu_down;
bool& g_stub_key_left = g_stub_state.stub_key_left;
bool& g_stub_key_right = g_stub_state.stub_key_right;
bool& g_stub_key_left_held = g_stub_state.stub_key_left_held;
bool& g_stub_key_right_held = g_stub_state.stub_key_right_held;
bool& g_escape_played_confirm = g_stub_state.escape_played_confirm;
int& g_begin_story_scene_calls = g_stub_state.begin_story_scene_calls;
int& g_story_scene_confirm_calls = g_stub_state.story_scene_confirm_calls;
int& g_audio_event_calls = g_stub_state.audio_event_calls;
int& g_audio_menu_move_calls = g_stub_state.audio_menu_move_calls;
int& g_audio_menu_confirm_calls = g_stub_state.audio_menu_confirm_calls;
bool& g_handle_escape_called = g_stub_state.handle_escape_called;
int& g_main_menu_input_calls = g_stub_state.main_menu_input_calls;
int& g_story_hub_input_calls = g_stub_state.story_hub_input_calls;
int& g_options_menu_input_calls = g_stub_state.options_menu_input_calls;
int& g_quick_menu_input_calls = g_stub_state.quick_menu_input_calls;
int& g_story_intro_input_calls = g_stub_state.story_intro_input_calls;
int& g_story_menu_input_calls = g_stub_state.story_menu_input_calls;
int& g_pause_input_calls = g_stub_state.pause_input_calls;
int& g_match_exit_policy_calls = g_stub_state.match_exit_policy_calls;
int& g_save_menu_settings_calls = g_stub_state.save_menu_settings_calls;
int& g_audio_set_settings_calls = g_stub_state.audio_set_settings_calls;
int& g_story_save_exists_calls = g_stub_state.story_save_exists_calls;
bool& g_story_save_exists_result = g_stub_state.story_save_exists_result;
bool& g_stub_options_changed_bindings = g_stub_state.stub_options_changed_bindings;
bool& g_stub_options_changed_audio_settings = g_stub_state.stub_options_changed_audio_settings;
bool& g_stub_main_menu_change_row = g_stub_state.stub_main_menu_change_row;
bool& g_stub_main_menu_change_state = g_stub_state.stub_main_menu_change_state;
bool& g_stub_options_change_row = g_stub_state.stub_options_change_row;
bool& g_stub_options_toggle_waiting = g_stub_state.stub_options_toggle_waiting;
bool& g_stub_options_change_state = g_stub_state.stub_options_change_state;
bool& g_stub_quick_menu_change_row = g_stub_state.stub_quick_menu_change_row;
bool& g_stub_quick_menu_change_options = g_stub_state.stub_quick_menu_change_options;
bool& g_stub_quick_menu_change_state = g_stub_state.stub_quick_menu_change_state;
whacker::app::AppState& g_stub_quick_menu_next_state = g_stub_state.stub_quick_menu_next_state;
bool& g_stub_story_intro_change_phase = g_stub_state.stub_story_intro_change_phase;
bool& g_stub_story_intro_toggle_accept = g_stub_state.stub_story_intro_toggle_accept;
bool& g_stub_story_intro_change_state = g_stub_state.stub_story_intro_change_state;
bool& g_stub_story_menu_move_selection = g_stub_state.stub_story_menu_move_selection;
bool& g_stub_story_menu_change_confirm_selected = g_stub_state.stub_story_menu_change_confirm_selected;
bool& g_stub_story_menu_change_state = g_stub_state.stub_story_menu_change_state;
bool& g_stub_story_menu_toggle_confirm_overwrite = g_stub_state.stub_story_menu_toggle_confirm_overwrite;
bool& g_stub_story_hub_change_row = g_stub_state.stub_story_hub_change_row;
bool& g_stub_story_hub_change_week = g_stub_state.stub_story_hub_change_week;
bool& g_stub_story_hub_change_state = g_stub_state.stub_story_hub_change_state;
whacker::app::AppState& g_stub_story_hub_next_state = g_stub_state.stub_story_hub_next_state;
bool& g_stub_pause_feedback_move = g_stub_state.stub_pause_feedback_move;
bool& g_stub_pause_feedback_confirm = g_stub_state.stub_pause_feedback_confirm;
int& g_story_save_career_calls = g_stub_state.story_save_career_calls;

[[maybe_unused]] bool approx_equal(const float a, const float b, const float eps = 1.0e-4f) {
    return std::fabs(a - b) <= eps;
}

void reset_stubs() {
    g_stub_state.reset();
}

struct RuntimeInputPhaseFixture {
    whacker::app::KeyEdgeState edge {};
    whacker::app::AppState app_state;
    whacker::app::AppState pause_return_state;
    bool show_dev_info = false;
    bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::AudioSettings audio_settings {};
    whacker::app::AudioEngine audio_engine {};
    whacker::app::MainMenuState main_menu {};
    whacker::app::OptionsMenuState options_menu {};
    whacker::app::PauseMenuState pause_menu {};
    whacker::app::MenuState quick_menu {};
    whacker::app::PaddleTuningState paddle_tuning {};
    whacker::app::StoryMenuState story_menu {};
    whacker::app::StoryIntroState story_intro {};
    whacker::app::StorySceneState story_scene {};
    whacker::app::StoryHubState story_hub {};
    whacker::app::StoryRuntimeState story_runtime {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng;

    RuntimeInputPhaseFixture(
        const whacker::app::AppState initial_app_state,
        const whacker::app::AppState initial_pause_return_state,
        const std::uint64_t seed)
        : app_state(initial_app_state),
          pause_return_state(initial_pause_return_state),
          rng(seed) {}

    void run(
        const double menu_input_lockout,
        const int story_official_games_to_win = 3,
        whacker::app::RuntimeStorySaveExistsCache* story_save_cache = nullptr) {
        whacker::app::handle_runtime_input_phase(
            nullptr,
            edge,
            app_state,
            pause_return_state,
            show_dev_info,
            ai_controls_player_paddle,
            options,
            controls,
            audio_settings,
            audio_engine,
            main_menu,
            options_menu,
            pause_menu,
            quick_menu,
            paddle_tuning,
            story_menu,
            story_intro,
            story_scene,
            story_hub,
            story_runtime,
            authored_transition_request,
            match_flow,
            simulation,
            rng,
            menu_input_lockout,
            story_official_games_to_win,
            story_save_cache);
    }
};

void test_story_scene_pending_gate_consumes_flags_once_and_is_idempotent() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::MainMenu,
        0xA11A11ULL};
    fixture.story_runtime.onboarding_scene_pending = true;
    fixture.story_runtime.post_forfeit_scene_pending = true;
    fixture.story_runtime.onboarding_step = whacker::app::StoryOnboardingStep::ClubIntroScene;

    fixture.run(1.0);

    TEST_CHECK(g_begin_story_scene_calls == 1);
    TEST_CHECK(!fixture.story_runtime.onboarding_scene_pending);
    TEST_CHECK(!fixture.story_runtime.post_forfeit_scene_pending);
    TEST_CHECK(fixture.story_scene.id == whacker::app::StorySceneId::PostForfeitSupport);
    TEST_CHECK(g_story_scene_confirm_calls == 0);

    fixture.run(1.0);

    TEST_CHECK(g_begin_story_scene_calls == 1);
    TEST_CHECK(fixture.story_scene.id == whacker::app::StorySceneId::PostForfeitSupport);
    TEST_CHECK(g_story_scene_confirm_calls == 0);
}

void test_story_scene_pending_gate_ignored_outside_story_scene_state() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryHub,
        whacker::app::AppState::MainMenu,
        0xB22B22ULL};
    fixture.story_runtime.onboarding_scene_pending = true;
    fixture.story_runtime.post_forfeit_scene_pending = true;

    fixture.run(1.0);

    TEST_CHECK(g_begin_story_scene_calls == 0);
    TEST_CHECK(fixture.story_runtime.onboarding_scene_pending);
    TEST_CHECK(fixture.story_runtime.post_forfeit_scene_pending);
    TEST_CHECK(fixture.story_scene.id == whacker::app::StorySceneId::None);
}

void test_story_scene_pending_gate_runs_before_story_scene_confirm_path() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::MainMenu,
        0xC33C33ULL};
    fixture.story_runtime.onboarding_scene_pending = true;
    fixture.story_runtime.post_forfeit_scene_pending = false;

    g_stub_confirm_press = true;
    fixture.run(0.0);

    TEST_CHECK(g_begin_story_scene_calls == 1);
    TEST_CHECK(!fixture.story_runtime.onboarding_scene_pending);
    TEST_CHECK(!fixture.story_runtime.post_forfeit_scene_pending);
    TEST_CHECK(g_story_scene_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_escape_with_confirm_feedback_emits_menu_confirm_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryHub,
        whacker::app::AppState::MainMenu,
        0xD44D44ULL};

    g_stub_escape = true;
    g_escape_played_confirm = true;
    fixture.run(1.0);

    TEST_CHECK(g_handle_escape_called);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_f10_toggles_dev_info_without_menu_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::MainMenu,
        whacker::app::AppState::MainMenu,
        0xE55E55ULL};

    g_stub_dev_info_key = true;
    fixture.run(1.0);

    TEST_CHECK(fixture.show_dev_info);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_dev_player_ai_toggle_requires_playing_surface_and_visible_dev_overlay() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::Playing,
        whacker::app::AppState::MainMenu,
        0xE66E66ULL};

    g_stub_dev_player_ai_key = true;
    fixture.run(1.0);

    TEST_CHECK(!fixture.ai_controls_player_paddle);
    TEST_CHECK(g_audio_event_calls == 0);

    fixture.show_dev_info = true;
    fixture.app_state = whacker::app::AppState::MainMenu;
    g_stub_dev_player_ai_key = true;
    fixture.run(1.0);

    TEST_CHECK(!fixture.ai_controls_player_paddle);
    TEST_CHECK(g_audio_event_calls == 0);

    fixture.app_state = whacker::app::AppState::Playing;
    g_stub_dev_player_ai_key = true;
    fixture.run(1.0);

    TEST_CHECK(fixture.ai_controls_player_paddle);
    TEST_CHECK(g_audio_event_calls == 0);

    g_stub_dev_player_ai_key = true;
    fixture.run(1.0);
    TEST_CHECK(!fixture.ai_controls_player_paddle);
    TEST_CHECK(g_audio_event_calls == 0);

    fixture.app_state = whacker::app::AppState::StoryIntro;
    fixture.story_intro.phase = whacker::app::StoryIntroPhase::Invite;
    g_stub_dev_player_ai_key = true;
    fixture.run(1.0);
    TEST_CHECK(!fixture.ai_controls_player_paddle);
    TEST_CHECK(g_audio_event_calls == 0);

    fixture.story_intro.phase = whacker::app::StoryIntroPhase::PlayMatch;
    g_stub_dev_player_ai_key = true;
    fixture.run(1.0);
    TEST_CHECK(fixture.ai_controls_player_paddle);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_menu_key_transitions_between_playing_and_quick_setup_when_eligible() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::Playing,
        whacker::app::AppState::MainMenu,
        0xF66F66ULL};
    fixture.story_runtime.active_match = whacker::app::StoryMatchKind::None;
    fixture.match_flow.mode = whacker::app::ActiveMatchMode::Quick;

    g_stub_menu_key = true;
    fixture.run(1.0);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::QuickMatchSetup);

    g_stub_menu_key = true;
    fixture.run(1.0);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
}

void test_menu_key_does_not_open_quick_setup_during_story_match() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::Playing,
        whacker::app::AppState::MainMenu,
        0x6A6A6AULL};
    fixture.story_runtime.active_match = whacker::app::StoryMatchKind::OnboardingAyaFriendly;
    fixture.match_flow.mode = whacker::app::ActiveMatchMode::Quick;

    g_stub_menu_key = true;
    fixture.run(1.0);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
}

void test_menu_key_does_not_leave_quick_setup_when_mode_not_quick() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::QuickMatchSetup,
        whacker::app::AppState::MainMenu,
        0x6B6B6BULL};
    fixture.match_flow.mode = whacker::app::ActiveMatchMode::StoryTraining;

    g_stub_menu_key = true;
    fixture.run(1.0);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::QuickMatchSetup);
}

void test_escape_and_f10_can_coexist_in_single_tick() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryMenu,
        whacker::app::AppState::MainMenu,
        0x707707ULL};

    g_stub_escape = true;
    g_escape_played_confirm = true;
    g_stub_dev_info_key = true;
    fixture.run(1.0);

    TEST_CHECK(g_handle_escape_called);
    TEST_CHECK(fixture.show_dev_info);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_story_scene_confirm_suppressed_when_lockout_positive_without_pending_gate() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::MainMenu,
        0x818181ULL};
    fixture.story_runtime.onboarding_scene_pending = false;
    fixture.story_runtime.post_forfeit_scene_pending = false;

    g_stub_confirm_press = true;
    fixture.run(0.5);

    TEST_CHECK(g_begin_story_scene_calls == 0);
    TEST_CHECK(g_story_scene_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_story_scene_confirm_runs_when_lockout_zero_without_pending_gate() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::MainMenu,
        0x919191ULL};
    fixture.story_runtime.onboarding_scene_pending = false;
    fixture.story_runtime.post_forfeit_scene_pending = false;

    g_stub_confirm_press = true;
    fixture.run(0.0);

    TEST_CHECK(g_begin_story_scene_calls == 0);
    TEST_CHECK(g_story_scene_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_main_menu_and_story_hub_handlers_respect_lockout_boundary() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::MainMenu,
        whacker::app::AppState::MainMenu,
        0xA2A2A2ULL};

    fixture.run(0.25);
    TEST_CHECK(g_main_menu_input_calls == 0);

    fixture.run(0.0);
    TEST_CHECK(g_main_menu_input_calls == 1);

    fixture.app_state = whacker::app::AppState::StoryHub;
    fixture.run(0.5);
    TEST_CHECK(g_story_hub_input_calls == 0);

    fixture.run(0.0);
    TEST_CHECK(g_story_hub_input_calls == 1);
}

void test_options_menu_handler_respects_lockout_boundary_and_side_effects() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::OptionsMenu,
        whacker::app::AppState::MainMenu,
        0xB3B3B3ULL};

    g_stub_options_changed_bindings = true;
    g_stub_options_changed_audio_settings = true;
    fixture.run(0.75);

    TEST_CHECK(g_options_menu_input_calls == 0);
    TEST_CHECK(g_audio_set_settings_calls == 0);
    TEST_CHECK(g_save_menu_settings_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);

    g_stub_options_changed_bindings = true;
    g_stub_options_changed_audio_settings = true;
    fixture.run(0.0);

    TEST_CHECK(g_options_menu_input_calls == 1);
    TEST_CHECK(g_audio_set_settings_calls == 1);
    TEST_CHECK(g_save_menu_settings_calls == 1);
    TEST_CHECK(g_audio_event_calls == 2);
}

void test_story_menu_handler_respects_lockout_boundary_and_feedback() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryMenu,
        whacker::app::AppState::MainMenu,
        0xC4C4C4ULL};

    g_stub_story_menu_move_selection = true;
    g_stub_story_menu_toggle_confirm_overwrite = true;
    fixture.run(0.5);

    TEST_CHECK(g_story_menu_input_calls == 0);
    TEST_CHECK(fixture.story_menu.selected_row == 0);
    TEST_CHECK(!fixture.story_menu.confirm_overwrite);
    TEST_CHECK(g_audio_event_calls == 0);

    g_stub_story_menu_move_selection = true;
    g_stub_story_menu_toggle_confirm_overwrite = true;
    fixture.run(0.0);

    TEST_CHECK(g_story_menu_input_calls == 1);
    TEST_CHECK(fixture.story_menu.selected_row == 1);
    TEST_CHECK(fixture.story_menu.confirm_overwrite);
    TEST_CHECK(g_audio_event_calls == 2);
}

void test_paused_handler_respects_lockout_boundary_and_feedback_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::Paused,
        whacker::app::AppState::Playing,
        0xD5D5D5ULL};

    g_stub_pause_feedback_move = true;
    g_stub_pause_feedback_confirm = true;
    fixture.run(0.6);

    TEST_CHECK(g_match_exit_policy_calls == 0);
    TEST_CHECK(g_pause_input_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);

    g_stub_pause_feedback_move = true;
    g_stub_pause_feedback_confirm = true;
    fixture.run(0.0);

    TEST_CHECK(g_match_exit_policy_calls == 1);
    TEST_CHECK(g_pause_input_calls == 1);
    TEST_CHECK(g_audio_event_calls == 2);
}

void test_quick_match_setup_handler_respects_lockout_boundary_and_side_effects() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::QuickMatchSetup,
        whacker::app::AppState::MainMenu,
        0xE6E6E6ULL};

    g_stub_quick_menu_change_row = true;
    g_stub_quick_menu_change_options = true;
    g_stub_quick_menu_change_state = true;
    fixture.run(0.4);

    TEST_CHECK(g_quick_menu_input_calls == 0);
    TEST_CHECK(g_save_menu_settings_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::QuickMatchSetup);
    TEST_CHECK(fixture.quick_menu.selected_row == 0);
    TEST_CHECK(fixture.options.left_mode == whacker::app::PaddleMode::Human);

    g_stub_quick_menu_change_row = true;
    g_stub_quick_menu_change_options = true;
    g_stub_quick_menu_change_state = true;
    fixture.run(0.0);

    TEST_CHECK(g_quick_menu_input_calls == 1);
    TEST_CHECK(g_save_menu_settings_calls == 1);
    TEST_CHECK(g_audio_event_calls == 2);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(fixture.quick_menu.selected_row == 1);
    TEST_CHECK(fixture.options.left_mode == whacker::app::PaddleMode::AI);
}

void test_story_intro_handler_respects_lockout_boundary_and_feedback() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryIntro,
        whacker::app::AppState::MainMenu,
        0xF7F7F7ULL};

    g_stub_story_intro_change_phase = true;
    g_stub_story_intro_toggle_accept = true;
    g_stub_story_intro_change_state = true;
    fixture.run(0.2);

    TEST_CHECK(g_story_intro_input_calls == 0);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(fixture.story_intro.phase == whacker::app::StoryIntroPhase::Invite);
    TEST_CHECK(!fixture.story_intro.name_accept_pending);
    TEST_CHECK(g_audio_event_calls == 0);

    g_stub_story_intro_change_phase = true;
    g_stub_story_intro_toggle_accept = true;
    g_stub_story_intro_change_state = true;
    fixture.run(0.0);

    TEST_CHECK(g_story_intro_input_calls == 1);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.story_intro.phase == whacker::app::StoryIntroPhase::RivalIntro);
    TEST_CHECK(fixture.story_intro.name_accept_pending);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_quick_match_setup_transition_to_paddle_tuning_initializes_quick_left_target() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::QuickMatchSetup,
        whacker::app::AppState::MainMenu,
        0xA1A2A3ULL};
    fixture.quick_menu.selected_row = whacker::app::MenuRowP1Tuning;
    fixture.options.left_paddle_skills = whacker::progression::SkillState {
        .edge = 0.20f,
        .power = 0.60f,
        .spin_inject = 0.20f};
    fixture.paddle_tuning.horizontal_hold_direction = 1;
    fixture.paddle_tuning.horizontal_hold_frames = 99;

    g_stub_quick_menu_change_state = true;
    g_stub_quick_menu_next_state = whacker::app::AppState::PaddleTuning;
    fixture.run(0.0);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::PaddleTuning);
    TEST_CHECK(fixture.paddle_tuning.active);
    TEST_CHECK(fixture.paddle_tuning.target == whacker::app::PaddleTuningTarget::QuickLeft);
    TEST_CHECK(fixture.paddle_tuning.return_state == whacker::app::AppState::QuickMatchSetup);
    TEST_CHECK(fixture.paddle_tuning.selected_component == 0);
    TEST_CHECK(fixture.paddle_tuning.horizontal_hold_direction == 0);
    TEST_CHECK(fixture.paddle_tuning.horizontal_hold_frames == 0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.20f));
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.power, 0.60f));
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.spin_inject, 0.20f));
}

void test_paddle_tuning_confirm_updates_quick_left_and_returns_to_quick_setup() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::PaddleTuning,
        whacker::app::AppState::MainMenu,
        0xA4A5A6ULL};
    fixture.paddle_tuning.active = true;
    fixture.paddle_tuning.return_state = whacker::app::AppState::QuickMatchSetup;
    fixture.paddle_tuning.target = whacker::app::PaddleTuningTarget::QuickLeft;
    fixture.paddle_tuning.working.edge = 0.70f;
    fixture.paddle_tuning.working.power = 0.20f;
    fixture.paddle_tuning.working.spin_inject = 0.10f;
    fixture.paddle_tuning.working.budget = 1.70f;

    g_stub_confirm_press = true;
    fixture.run(0.0);

    const whacker::progression::SkillState expected =
        whacker::app::paddle_tuning_to_skills(fixture.paddle_tuning.working);
    static_cast<void>(expected);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::QuickMatchSetup);
    TEST_CHECK(!fixture.paddle_tuning.active);
    TEST_CHECK(approx_equal(fixture.options.left_paddle_skills.edge, expected.edge));
    TEST_CHECK(approx_equal(fixture.options.left_paddle_skills.power, expected.power));
    TEST_CHECK(approx_equal(fixture.options.left_paddle_skills.spin_inject, expected.spin_inject));
    TEST_CHECK(fixture.options.left_ai_style == whacker::app::style_for_skills(expected));
    TEST_CHECK(g_save_menu_settings_calls == 1);
    TEST_CHECK(g_story_save_career_calls == 0);
}

void test_story_hub_transition_to_paddle_tuning_initializes_story_player_target() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryHub,
        whacker::app::AppState::MainMenu,
        0xA7A8A9ULL};
    fixture.story_hub.selected_row = whacker::app::StoryHubRowPaddleTuning;
    fixture.story_runtime.career.player_skills = whacker::progression::SkillState {
        .edge = 0.55f,
        .power = 0.25f,
        .spin_inject = 0.20f};
    fixture.story_runtime.career.player_skill_caps = whacker::progression::SkillState {
        .edge = 0.60f,
        .power = 0.30f,
        .spin_inject = 0.25f};
    fixture.paddle_tuning.horizontal_hold_direction = -1;
    fixture.paddle_tuning.horizontal_hold_frames = 88;

    g_stub_story_hub_change_state = true;
    g_stub_story_hub_next_state = whacker::app::AppState::PaddleTuning;
    fixture.run(0.0);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::PaddleTuning);
    TEST_CHECK(fixture.paddle_tuning.active);
    TEST_CHECK(fixture.paddle_tuning.target == whacker::app::PaddleTuningTarget::StoryPlayer);
    TEST_CHECK(fixture.paddle_tuning.return_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(fixture.paddle_tuning.selected_component == 0);
    TEST_CHECK(fixture.paddle_tuning.horizontal_hold_direction == 0);
    TEST_CHECK(fixture.paddle_tuning.horizontal_hold_frames == 0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.55f));
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.power, 0.25f));
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.spin_inject, 0.20f));
    TEST_CHECK(approx_equal(fixture.paddle_tuning.max_skills.edge, 0.60f));
    TEST_CHECK(approx_equal(fixture.paddle_tuning.max_skills.power, 0.30f));
    TEST_CHECK(approx_equal(fixture.paddle_tuning.max_skills.spin_inject, 0.25f));
    TEST_CHECK(approx_equal(fixture.paddle_tuning.max_budget, 1.15f));
}

void test_paddle_tuning_confirm_updates_story_player_and_returns_to_story_hub() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::PaddleTuning,
        whacker::app::AppState::MainMenu,
        0xAAABACULL};
    fixture.paddle_tuning.active = true;
    fixture.paddle_tuning.return_state = whacker::app::AppState::StoryHub;
    fixture.paddle_tuning.target = whacker::app::PaddleTuningTarget::StoryPlayer;
    fixture.story_runtime.career.player_skill_caps = whacker::progression::SkillState {
        .edge = 0.80f,
        .power = 0.30f,
        .spin_inject = 0.60f};
    fixture.paddle_tuning.max_skills = fixture.story_runtime.career.player_skill_caps;
    fixture.paddle_tuning.max_budget = 1.70f;
    fixture.paddle_tuning.working.edge = 0.25f;
    fixture.paddle_tuning.working.power = 0.15f;
    fixture.paddle_tuning.working.spin_inject = 0.60f;
    fixture.paddle_tuning.working.budget = 1.40f;

    g_stub_confirm_press = true;
    fixture.run(0.0);

    const whacker::progression::SkillState expected =
        whacker::app::paddle_tuning_to_skills(fixture.paddle_tuning.working);
    static_cast<void>(expected);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(!fixture.paddle_tuning.active);
    TEST_CHECK(approx_equal(fixture.story_runtime.career.player_skills.edge, expected.edge));
    TEST_CHECK(approx_equal(fixture.story_runtime.career.player_skills.power, expected.power));
    TEST_CHECK(approx_equal(fixture.story_runtime.career.player_skills.spin_inject, expected.spin_inject));
    TEST_CHECK(g_story_save_career_calls == 1);
    TEST_CHECK(g_save_menu_settings_calls == 0);
}

void test_story_player_paddle_tuning_bar_adjustment_respects_skill_caps() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::PaddleTuning,
        whacker::app::AppState::MainMenu,
        0xAABBEEULL};
    fixture.paddle_tuning.active = true;
    fixture.paddle_tuning.return_state = whacker::app::AppState::StoryHub;
    fixture.paddle_tuning.target = whacker::app::PaddleTuningTarget::StoryPlayer;
    fixture.story_runtime.career.player_skill_caps = whacker::progression::SkillState {
        .edge = 0.30f,
        .power = 0.20f,
        .spin_inject = 0.10f};
    fixture.paddle_tuning.max_skills = fixture.story_runtime.career.player_skill_caps;
    fixture.paddle_tuning.max_budget = 0.60f;
    fixture.paddle_tuning.working.edge = 0.30f;
    fixture.paddle_tuning.working.power = 0.20f;
    fixture.paddle_tuning.working.spin_inject = 0.10f;
    fixture.paddle_tuning.working.budget = 0.60f;
    fixture.paddle_tuning.selected_component = 0;

    g_stub_key_right = true;
    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.30f));
    TEST_CHECK(g_audio_menu_move_calls == 0);

    fixture.paddle_tuning.selected_component = 1;
    g_stub_key_left = true;
    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.power, 0.19f));
    TEST_CHECK(g_audio_menu_move_calls == 1);

    fixture.paddle_tuning.selected_component = 2;
    g_stub_key_right = true;
    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.spin_inject, 0.10f));
    TEST_CHECK(g_audio_menu_move_calls == 1);
}

void test_story_player_paddle_tuning_confirm_clamps_to_skill_caps() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::PaddleTuning,
        whacker::app::AppState::MainMenu,
        0xEECCAAULL};
    fixture.paddle_tuning.active = true;
    fixture.paddle_tuning.return_state = whacker::app::AppState::StoryHub;
    fixture.paddle_tuning.target = whacker::app::PaddleTuningTarget::StoryPlayer;
    fixture.story_runtime.career.player_skill_caps = whacker::progression::SkillState {
        .edge = 0.30f,
        .power = 0.20f,
        .spin_inject = 0.10f};
    fixture.paddle_tuning.max_skills = fixture.story_runtime.career.player_skill_caps;
    fixture.paddle_tuning.max_budget = 0.60f;
    fixture.paddle_tuning.working.edge = 0.80f;
    fixture.paddle_tuning.working.power = 0.70f;
    fixture.paddle_tuning.working.spin_inject = 0.40f;
    fixture.paddle_tuning.working.budget = 1.70f;

    g_stub_confirm_press = true;
    fixture.run(0.0);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(!fixture.paddle_tuning.active);
    TEST_CHECK(approx_equal(fixture.story_runtime.career.player_skills.edge, 0.30f));
    TEST_CHECK(approx_equal(fixture.story_runtime.career.player_skills.power, 0.20f));
    TEST_CHECK(approx_equal(fixture.story_runtime.career.player_skills.spin_inject, 0.10f));
    TEST_CHECK(g_story_save_career_calls == 1);
    TEST_CHECK(g_save_menu_settings_calls == 0);
}

void test_paddle_tuning_component_selection_wraps_with_up_down() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::PaddleTuning,
        whacker::app::AppState::MainMenu,
        0xADADADULL};
    fixture.paddle_tuning.selected_component = 0;

    g_stub_menu_up = true;
    fixture.run(0.0);

    TEST_CHECK(fixture.paddle_tuning.selected_component == 2);
    TEST_CHECK(g_audio_menu_move_calls == 1);

    g_stub_menu_down = true;
    fixture.run(0.0);

    TEST_CHECK(fixture.paddle_tuning.selected_component == 0);
    TEST_CHECK(g_audio_menu_move_calls == 2);
}

void test_paddle_tuning_bar_adjustment_respects_budget_cap_and_noop_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::PaddleTuning,
        whacker::app::AppState::MainMenu,
        0xAEAEAEULL};
    fixture.paddle_tuning.working.edge = 1.0f;
    fixture.paddle_tuning.working.power = 0.70f;
    fixture.paddle_tuning.working.spin_inject = 0.0f;
    fixture.paddle_tuning.working.budget = 0.0f;
    fixture.paddle_tuning.selected_component = 2;

    g_stub_key_right = true;
    fixture.run(0.0);

    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.spin_inject, 0.0f));
    TEST_CHECK(g_audio_menu_move_calls == 0);

    fixture.paddle_tuning.selected_component = 1;
    g_stub_key_left = true;
    fixture.run(0.0);

    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.power, 0.69f));
    TEST_CHECK(g_audio_menu_move_calls == 1);

    fixture.paddle_tuning.selected_component = 2;
    g_stub_key_right = true;
    fixture.run(0.0);

    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.spin_inject, 0.01f));
    TEST_CHECK((fixture.paddle_tuning.working.edge +
            fixture.paddle_tuning.working.power +
            fixture.paddle_tuning.working.spin_inject) <= 1.7001f);
    TEST_CHECK(g_audio_menu_move_calls == 2);
}

void test_paddle_tuning_bar_hold_repeat_applies_after_delay() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::PaddleTuning,
        whacker::app::AppState::MainMenu,
        0xAFAFAFULL};
    fixture.paddle_tuning.working.edge = 0.50f;
    fixture.paddle_tuning.working.power = 0.20f;
    fixture.paddle_tuning.working.spin_inject = 0.20f;
    fixture.paddle_tuning.working.budget = 0.90f;
    fixture.paddle_tuning.selected_component = 0;

    g_stub_key_right = true;
    g_stub_key_right_held = true;
    fixture.run(0.0);

    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.51f));
    TEST_CHECK(g_audio_menu_move_calls == 1);

    for (int i = 0; i < whacker::app::kPaddleTuningHoldRepeatDelayFrames - 1; ++i) {
        fixture.run(0.0);
    }

    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.51f));
    TEST_CHECK(g_audio_menu_move_calls == 1);

    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.52f));
    TEST_CHECK(g_audio_menu_move_calls == 2);

    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.52f));
    TEST_CHECK(g_audio_menu_move_calls == 2);

    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.53f));
    TEST_CHECK(g_audio_menu_move_calls == 3);

    g_stub_key_right_held = false;
    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.53f));
    TEST_CHECK(g_audio_menu_move_calls == 3);
}

void test_paddle_tuning_hold_repeat_stops_when_both_horizontal_keys_down() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::PaddleTuning,
        whacker::app::AppState::MainMenu,
        0xB0B0B0ULL};
    fixture.paddle_tuning.working.edge = 0.50f;
    fixture.paddle_tuning.working.power = 0.20f;
    fixture.paddle_tuning.working.spin_inject = 0.20f;
    fixture.paddle_tuning.working.budget = 0.90f;
    fixture.paddle_tuning.selected_component = 0;

    g_stub_key_right = true;
    g_stub_key_right_held = true;
    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.51f));

    for (int i = 0; i < whacker::app::kPaddleTuningHoldRepeatDelayFrames; ++i) {
        fixture.run(0.0);
    }
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.52f));

    g_stub_key_left_held = true;
    fixture.run(0.0);
    fixture.run(0.0);
    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.52f));

    g_stub_key_left_held = false;
    fixture.run(0.0);
    TEST_CHECK(approx_equal(fixture.paddle_tuning.working.edge, 0.52f));
}

void test_main_menu_row_change_emits_move_without_confirm_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::MainMenu,
        whacker::app::AppState::MainMenu,
        0x112233ULL};

    g_stub_main_menu_change_row = true;
    fixture.run(0.0);

    TEST_CHECK(g_main_menu_input_calls == 1);
    TEST_CHECK(fixture.main_menu.selected_row == 1);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_main_menu_state_change_emits_confirm_without_move_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::MainMenu,
        whacker::app::AppState::MainMenu,
        0x223344ULL};

    g_stub_main_menu_change_state = true;
    fixture.run(0.0);

    TEST_CHECK(g_main_menu_input_calls == 1);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryMenu);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_options_audio_settings_change_emits_move_and_save_without_confirm_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::OptionsMenu,
        whacker::app::AppState::MainMenu,
        0x334455ULL};

    g_stub_options_changed_audio_settings = true;
    fixture.run(0.0);

    TEST_CHECK(g_options_menu_input_calls == 1);
    TEST_CHECK(g_audio_set_settings_calls == 1);
    TEST_CHECK(g_save_menu_settings_calls == 1);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_options_binding_change_emits_confirm_and_save_without_move_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::OptionsMenu,
        whacker::app::AppState::MainMenu,
        0x445566ULL};

    g_stub_options_changed_bindings = true;
    fixture.run(0.0);

    TEST_CHECK(g_options_menu_input_calls == 1);
    TEST_CHECK(g_audio_set_settings_calls == 0);
    TEST_CHECK(g_save_menu_settings_calls == 1);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_options_row_and_audio_change_emit_two_move_events() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::OptionsMenu,
        whacker::app::AppState::MainMenu,
        0x556688ULL};

    g_stub_options_change_row = true;
    g_stub_options_changed_audio_settings = true;
    fixture.run(0.0);

    TEST_CHECK(g_options_menu_input_calls == 1);
    TEST_CHECK(g_audio_set_settings_calls == 1);
    TEST_CHECK(g_save_menu_settings_calls == 1);
    TEST_CHECK(g_audio_menu_move_calls == 2);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 2);
}

void test_story_menu_confirm_selection_change_emits_move_without_confirm_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryMenu,
        whacker::app::AppState::MainMenu,
        0x556677ULL};

    g_stub_story_menu_change_confirm_selected = true;
    fixture.run(0.0);

    TEST_CHECK(g_story_menu_input_calls == 1);
    TEST_CHECK(fixture.story_menu.confirm_selected == 1);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_story_hub_row_change_emits_move_without_confirm_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryHub,
        whacker::app::AppState::MainMenu,
        0x667788ULL};
    fixture.story_runtime.career.week = 3;

    g_stub_story_hub_change_row = true;
    fixture.run(0.0);

    TEST_CHECK(g_story_hub_input_calls == 1);
    TEST_CHECK(fixture.story_hub.selected_row == 1);
    TEST_CHECK(fixture.story_runtime.career.week == 3);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_story_hub_week_change_emits_confirm_without_move_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryHub,
        whacker::app::AppState::MainMenu,
        0x778899ULL};
    fixture.story_runtime.career.week = 5;

    g_stub_story_hub_change_week = true;
    fixture.run(0.0);

    TEST_CHECK(g_story_hub_input_calls == 1);
    TEST_CHECK(fixture.story_hub.selected_row == 0);
    TEST_CHECK(fixture.story_runtime.career.week == 6);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_main_menu_no_mutation_emits_no_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::MainMenu,
        whacker::app::AppState::MainMenu,
        0x8899AAULL};
    fixture.main_menu.selected_row = 2;

    fixture.run(0.0);

    TEST_CHECK(g_main_menu_input_calls == 1);
    TEST_CHECK(fixture.main_menu.selected_row == 2);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_options_menu_no_mutation_emits_no_audio_or_save() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::OptionsMenu,
        whacker::app::AppState::MainMenu,
        0x99AABBULL};
    fixture.options_menu.selected_row = 3;
    fixture.options_menu.waiting_for_key = true;

    fixture.run(0.0);

    TEST_CHECK(g_options_menu_input_calls == 1);
    TEST_CHECK(fixture.options_menu.selected_row == 3);
    TEST_CHECK(fixture.options_menu.waiting_for_key);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::OptionsMenu);
    TEST_CHECK(g_audio_set_settings_calls == 0);
    TEST_CHECK(g_save_menu_settings_calls == 0);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_quick_menu_no_mutation_emits_no_audio_or_save() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::QuickMatchSetup,
        whacker::app::AppState::MainMenu,
        0xAABBCCULL};
    fixture.options.left_mode = whacker::app::PaddleMode::Human;
    fixture.options.right_mode = whacker::app::PaddleMode::AI;
    fixture.quick_menu.selected_row = 2;

    fixture.run(0.0);

    TEST_CHECK(g_quick_menu_input_calls == 1);
    TEST_CHECK(fixture.quick_menu.selected_row == 2);
    TEST_CHECK(fixture.options.left_mode == whacker::app::PaddleMode::Human);
    TEST_CHECK(fixture.options.right_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::QuickMatchSetup);
    TEST_CHECK(g_save_menu_settings_calls == 0);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_story_menu_no_mutation_emits_no_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryMenu,
        whacker::app::AppState::MainMenu,
        0xBBCCDDULL};
    fixture.story_menu.selected_row = 1;
    fixture.story_menu.confirm_selected = 1;
    fixture.story_menu.confirm_overwrite = true;

    fixture.run(0.0);

    TEST_CHECK(g_story_menu_input_calls == 1);
    TEST_CHECK(fixture.story_menu.selected_row == 1);
    TEST_CHECK(fixture.story_menu.confirm_selected == 1);
    TEST_CHECK(fixture.story_menu.confirm_overwrite);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryMenu);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_story_intro_no_mutation_emits_no_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryIntro,
        whacker::app::AppState::MainMenu,
        0xCCDDEEULL};
    fixture.story_intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    fixture.story_intro.break_kind = whacker::app::StoryIntroBreak::Rules;
    fixture.story_intro.name_accept_pending = true;

    fixture.run(0.0);

    TEST_CHECK(g_story_intro_input_calls == 1);
    TEST_CHECK(fixture.story_intro.phase == whacker::app::StoryIntroPhase::RivalIntro);
    TEST_CHECK(fixture.story_intro.break_kind == whacker::app::StoryIntroBreak::Rules);
    TEST_CHECK(fixture.story_intro.name_accept_pending);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_story_hub_no_mutation_emits_no_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryHub,
        whacker::app::AppState::MainMenu,
        0xDDEEFFULL};
    fixture.story_hub.selected_row = 2;
    fixture.story_runtime.career.week = 7;

    fixture.run(0.0);

    TEST_CHECK(g_story_hub_input_calls == 1);
    TEST_CHECK(fixture.story_hub.selected_row == 2);
    TEST_CHECK(fixture.story_runtime.career.week == 7);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_paused_no_feedback_emits_no_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::Paused,
        whacker::app::AppState::Playing,
        0xEEFF11ULL};

    fixture.run(0.0);

    TEST_CHECK(g_match_exit_policy_calls == 1);
    TEST_CHECK(g_pause_input_calls == 1);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_story_scene_no_confirm_input_emits_no_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::MainMenu,
        0xFF1122ULL};
    fixture.story_runtime.onboarding_scene_pending = false;
    fixture.story_runtime.post_forfeit_scene_pending = false;

    fixture.run(0.0);

    TEST_CHECK(g_story_scene_confirm_calls == 0);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_story_scene_scroll_up_down_uses_menu_vertical_input() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::MainMenu,
        0x102030ULL};
    fixture.story_runtime.onboarding_scene_pending = false;
    fixture.story_runtime.post_forfeit_scene_pending = false;
    fixture.story_scene.id = whacker::app::StorySceneId::OnboardingClubIntro;
    fixture.story_scene.line_count = 1;
    fixture.story_scene.line_index = 0;
    fixture.story_scene.lines[0] =
        "AYA: LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG "
        "LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG "
        "LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG "
        "LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG.";
    fixture.story_scene.visible_chars = fixture.story_scene.lines[0].size();
    fixture.story_scene.dialogue_writing = false;
    fixture.story_scene.scroll_lines_from_bottom = 0;

    g_stub_menu_up = true;
    fixture.run(0.0);

    TEST_CHECK(fixture.story_scene.scroll_lines_from_bottom > 0);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_story_scene_confirm_calls == 0);

    g_stub_menu_down = true;
    fixture.run(0.0);

    TEST_CHECK(fixture.story_scene.scroll_lines_from_bottom == 0);
    TEST_CHECK(g_audio_menu_move_calls == 2);
    TEST_CHECK(g_audio_menu_confirm_calls == 0);
    TEST_CHECK(g_story_scene_confirm_calls == 0);
}

void test_story_scene_confirm_snaps_to_latest_before_advancing() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::MainMenu,
        0x112131ULL};
    fixture.story_runtime.onboarding_scene_pending = false;
    fixture.story_runtime.post_forfeit_scene_pending = false;
    fixture.story_scene.id = whacker::app::StorySceneId::OnboardingClubIntro;
    fixture.story_scene.line_count = 1;
    fixture.story_scene.line_index = 0;
    fixture.story_scene.lines[0] =
        "AYA: LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG "
        "LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG "
        "LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG "
        "LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG.";
    fixture.story_scene.visible_chars = fixture.story_scene.lines[0].size();
    fixture.story_scene.dialogue_writing = false;
    fixture.story_scene.scroll_lines_from_bottom = 0;

    g_stub_menu_up = true;
    fixture.run(0.0);
    TEST_CHECK(fixture.story_scene.scroll_lines_from_bottom > 0);

    g_stub_confirm_press = true;
    fixture.run(0.0);

    TEST_CHECK(fixture.story_scene.scroll_lines_from_bottom == 0);
    TEST_CHECK(g_story_scene_confirm_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);

    g_stub_confirm_press = true;
    fixture.run(0.0);

    TEST_CHECK(g_story_scene_confirm_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 2);
}

void test_story_scene_binary_choice_uses_left_right_not_up_down() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::MainMenu,
        0x132435ULL};
    fixture.story_runtime.onboarding_scene_pending = false;
    fixture.story_runtime.post_forfeit_scene_pending = false;
    fixture.story_scene.id = whacker::app::StorySceneId::TixMidweekLunchInvite;
    fixture.story_scene.line_count = 1;
    fixture.story_scene.line_index = 0;
    fixture.story_scene.lines[0] = "TIX: ONE GAME?";
    fixture.story_scene.visible_chars = fixture.story_scene.lines[0].size();
    fixture.story_scene.dialogue_writing = false;
    fixture.story_scene.has_binary_choice = true;
    fixture.story_scene.binary_choice_yes_selected = true;

    g_stub_menu_up = true;
    fixture.run(0.0);

    TEST_CHECK(fixture.story_scene.binary_choice_yes_selected);
    TEST_CHECK(g_audio_menu_move_calls == 0);

    g_stub_key_left = true;
    fixture.run(0.0);

    TEST_CHECK(!fixture.story_scene.binary_choice_yes_selected);
    TEST_CHECK(g_audio_menu_move_calls == 1);
}

void test_main_menu_combined_mutations_emit_expected_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::MainMenu,
        whacker::app::AppState::MainMenu,
        0x101112ULL};

    g_stub_main_menu_change_row = true;
    g_stub_main_menu_change_state = true;
    fixture.run(0.0);

    TEST_CHECK(g_main_menu_input_calls == 1);
    TEST_CHECK(fixture.main_menu.selected_row == 1);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryMenu);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 2);
}

void test_options_combined_mutations_emit_expected_audio_and_side_effects() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::OptionsMenu,
        whacker::app::AppState::MainMenu,
        0x121314ULL};

    g_stub_options_change_row = true;
    g_stub_options_toggle_waiting = true;
    g_stub_options_change_state = true;
    g_stub_options_changed_bindings = true;
    g_stub_options_changed_audio_settings = true;
    fixture.run(0.0);

    TEST_CHECK(g_options_menu_input_calls == 1);
    TEST_CHECK(fixture.options_menu.selected_row == 1);
    TEST_CHECK(fixture.options_menu.waiting_for_key);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(g_audio_set_settings_calls == 1);
    TEST_CHECK(g_save_menu_settings_calls == 1);
    TEST_CHECK(g_audio_menu_move_calls == 2);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 3);
}

void test_quick_menu_combined_mutations_emit_expected_audio_and_save() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::QuickMatchSetup,
        whacker::app::AppState::MainMenu,
        0x141516ULL};

    g_stub_quick_menu_change_row = true;
    g_stub_quick_menu_change_options = true;
    g_stub_quick_menu_change_state = true;
    fixture.run(0.0);

    TEST_CHECK(g_quick_menu_input_calls == 1);
    TEST_CHECK(fixture.quick_menu.selected_row == 1);
    TEST_CHECK(fixture.options.left_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(g_save_menu_settings_calls == 1);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 2);
}

void test_story_menu_combined_mutations_emit_expected_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryMenu,
        whacker::app::AppState::MainMenu,
        0x161718ULL};

    g_stub_story_menu_move_selection = true;
    g_stub_story_menu_change_confirm_selected = true;
    g_stub_story_menu_toggle_confirm_overwrite = true;
    g_stub_story_menu_change_state = true;
    fixture.run(0.0);

    TEST_CHECK(g_story_menu_input_calls == 1);
    TEST_CHECK(fixture.story_menu.selected_row == 1);
    TEST_CHECK(fixture.story_menu.confirm_selected == 1);
    TEST_CHECK(fixture.story_menu.confirm_overwrite);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 2);
}

void test_story_hub_combined_mutations_emit_expected_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryHub,
        whacker::app::AppState::MainMenu,
        0x18191AULL};
    fixture.story_runtime.career.week = 10;

    g_stub_story_hub_change_row = true;
    g_stub_story_hub_change_week = true;
    g_stub_story_hub_change_state = true;
    fixture.run(0.0);

    TEST_CHECK(g_story_hub_input_calls == 1);
    TEST_CHECK(fixture.story_hub.selected_row == 1);
    TEST_CHECK(fixture.story_runtime.career.week == 11);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 2);
}

void test_story_intro_combined_mutations_emit_single_confirm_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryIntro,
        whacker::app::AppState::MainMenu,
        0x1A1B1CULL};

    g_stub_story_intro_change_phase = true;
    g_stub_story_intro_toggle_accept = true;
    g_stub_story_intro_change_state = true;
    fixture.run(0.0);

    TEST_CHECK(g_story_intro_input_calls == 1);
    TEST_CHECK(fixture.story_intro.phase == whacker::app::StoryIntroPhase::RivalIntro);
    TEST_CHECK(fixture.story_intro.name_accept_pending);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(g_audio_menu_move_calls == 0);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 1);
}

void test_paused_combined_feedback_emits_move_and_confirm_audio() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::Paused,
        whacker::app::AppState::Playing,
        0x1C1D1EULL};

    g_stub_pause_feedback_move = true;
    g_stub_pause_feedback_confirm = true;
    fixture.run(0.0);

    TEST_CHECK(g_match_exit_policy_calls == 1);
    TEST_CHECK(g_pause_input_calls == 1);
    TEST_CHECK(g_audio_menu_move_calls == 1);
    TEST_CHECK(g_audio_menu_confirm_calls == 1);
    TEST_CHECK(g_audio_event_calls == 2);
}

void test_story_menu_save_exists_resolution_respects_lockout_and_cache() {
    reset_stubs();

    RuntimeInputPhaseFixture fixture {
        whacker::app::AppState::StoryMenu,
        whacker::app::AppState::MainMenu,
        0x1E1F20ULL};
    whacker::app::RuntimeStorySaveExistsCache cache {};
    g_story_save_exists_result = true;

    fixture.run(1.0, 3, &cache);
    TEST_CHECK(g_story_menu_input_calls == 0);
    TEST_CHECK(g_story_save_exists_calls == 0);

    fixture.run(0.0, 3, &cache);
    TEST_CHECK(g_story_menu_input_calls == 1);
    TEST_CHECK(g_story_save_exists_calls == 1);

    fixture.run(0.0, 3, &cache);
    TEST_CHECK(g_story_menu_input_calls == 2);
    TEST_CHECK(g_story_save_exists_calls == 1);
}

}  // namespace

namespace whacker::app {

AudioEngine::~AudioEngine() = default;

bool AudioEngine::init() {
    return true;
}

void AudioEngine::shutdown() {}

bool AudioEngine::available() const {
    return true;
}

void AudioEngine::set_settings(const AudioSettings& /*settings*/) {
    ++g_audio_set_settings_calls;
}

AudioSettings AudioEngine::settings() const {
    return AudioSettings {};
}

void AudioEngine::push_event(const AudioEventId event_id) {
    ++g_audio_event_calls;
    if (event_id == AudioEventId::MenuMove) {
        ++g_audio_menu_move_calls;
    } else if (event_id == AudioEventId::MenuConfirm) {
        ++g_audio_menu_confirm_calls;
    }
}

void AudioEngine::push_paddle_hit(const PaddleHitAudioParams& /*params*/) {}

void AudioEngine::push_wall_hit(const WallHitAudioParams& /*params*/) {}

AudioSettings clamp_audio_settings(const AudioSettings settings) {
    return settings;
}

bool consume_key_press(GLFWwindow* /*window*/, const int key, bool& previous_down) {
    bool pressed = false;
    if (key == GLFW_KEY_LEFT) {
        pressed = g_stub_key_left;
        g_stub_key_left = false;
        previous_down = pressed || g_stub_key_left_held;
        return pressed;
    } else if (key == GLFW_KEY_RIGHT) {
        pressed = g_stub_key_right;
        g_stub_key_right = false;
        previous_down = pressed || g_stub_key_right_held;
        return pressed;
    } else if (key == GLFW_KEY_ESCAPE) {
        pressed = g_stub_escape;
        g_stub_escape = false;
    } else if (key == GLFW_KEY_M) {
        pressed = g_stub_menu_key;
        g_stub_menu_key = false;
    } else if (key == GLFW_KEY_F10) {
        pressed = g_stub_dev_info_key;
        g_stub_dev_info_key = false;
    } else if (key == GLFW_KEY_P) {
        pressed = g_stub_dev_player_ai_key;
        g_stub_dev_player_ai_key = false;
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

bool consume_menu_down_press(
    GLFWwindow* /*window*/,
    KeyEdgeState& /*edge_state*/,
    const ControlBindings& /*controls*/) {
    const bool pressed = g_stub_menu_down;
    g_stub_menu_down = false;
    return pressed;
}

bool handle_runtime_escape_key(
    GLFWwindow* /*window*/,
    AppState& /*app_state*/,
    AppState& /*pause_return_state*/,
    StoryMenuState& /*story_menu_state*/,
    OptionsMenuState& /*options_menu_state*/,
    PauseMenuState& /*pause_menu_state*/,
    StorySceneState& /*story_scene_state*/,
    PaddleTuningState& /*paddle_tuning_state*/,
    StoryRuntimeState& /*story_runtime*/) {
    g_handle_escape_called = true;
    return g_escape_played_confirm;
}

void begin_story_onboarding_scene(
    StorySceneState& story_scene_state,
    const StoryRuntimeState& story_runtime) {
    ++g_begin_story_scene_calls;
    story_scene_state.id = story_runtime.post_forfeit_scene_pending
        ? StorySceneId::PostForfeitSupport
        : StorySceneId::OnboardingClubIntro;
}

void handle_main_menu_input(
    GLFWwindow* /*window*/,
    KeyEdgeState& /*edge_state*/,
    MainMenuState& main_menu_state,
    MenuState& /*quick_menu_state*/,
    StoryMenuState& /*story_menu_state*/,
    OptionsMenuState& /*options_menu_state*/,
    const ControlBindings& /*controls*/,
    AppState& app_state) {
    ++g_main_menu_input_calls;
    if (g_stub_main_menu_change_row) {
        ++main_menu_state.selected_row;
        g_stub_main_menu_change_row = false;
    }
    if (g_stub_main_menu_change_state) {
        app_state = AppState::StoryMenu;
        g_stub_main_menu_change_state = false;
    }
}

void handle_options_menu_input(
    GLFWwindow* /*window*/,
    KeyEdgeState& /*edge_state*/,
    OptionsMenuState& options_menu_state,
    ControlBindings& /*controls*/,
    AudioSettings& /*audio_settings*/,
    AppState& app_state,
    bool& changed_bindings,
    bool& changed_audio_settings) {
    ++g_options_menu_input_calls;
    if (g_stub_options_change_row) {
        ++options_menu_state.selected_row;
        g_stub_options_change_row = false;
    }
    if (g_stub_options_toggle_waiting) {
        options_menu_state.waiting_for_key = !options_menu_state.waiting_for_key;
        g_stub_options_toggle_waiting = false;
    }
    if (g_stub_options_change_state) {
        app_state = AppState::MainMenu;
        g_stub_options_change_state = false;
    }
    changed_bindings = g_stub_options_changed_bindings;
    changed_audio_settings = g_stub_options_changed_audio_settings;
    g_stub_options_changed_bindings = false;
    g_stub_options_changed_audio_settings = false;
}

void save_menu_settings(
    const MatchOptions& /*options*/,
    const ControlBindings& /*controls*/,
    const AudioSettings& /*audio_settings*/) {
    ++g_save_menu_settings_calls;
}

void handle_menu_input(
    GLFWwindow* /*window*/,
    KeyEdgeState& /*edge_state*/,
    MenuState& menu_state,
    MatchOptions& options,
    const ControlBindings& /*controls*/,
    MatchFlowState& /*match_flow*/,
    AppState& app_state,
    whacker::sim::Simulation& /*simulation*/,
    std::mt19937_64& /*rng*/) {
    ++g_quick_menu_input_calls;
    if (g_stub_quick_menu_change_row) {
        ++menu_state.selected_row;
        g_stub_quick_menu_change_row = false;
    }
    if (g_stub_quick_menu_change_options) {
        options.left_mode = options.left_mode == PaddleMode::Human
            ? PaddleMode::AI
            : PaddleMode::Human;
        g_stub_quick_menu_change_options = false;
    }
    if (g_stub_quick_menu_change_state) {
        app_state = g_stub_quick_menu_next_state;
        g_stub_quick_menu_change_state = false;
    }
}

void handle_story_menu_input(
    GLFWwindow* /*window*/,
    KeyEdgeState& /*edge_state*/,
    StoryMenuState& story_menu_state,
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    StoryIntroState& /*story_intro_state*/,
    MatchOptions& /*options*/,
    const ControlBindings& /*controls*/,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    AppState& app_state,
    const bool /*has_save*/,
    const StoryLoadCareerFn /*load_career_fn*/,
    const StoryResetCareerFn /*reset_career_fn*/) {
    ++g_story_menu_input_calls;
    if (g_stub_story_menu_move_selection) {
        ++story_menu_state.selected_row;
        g_stub_story_menu_move_selection = false;
    }
    if (g_stub_story_menu_change_confirm_selected) {
        ++story_menu_state.confirm_selected;
        g_stub_story_menu_change_confirm_selected = false;
    }
    if (g_stub_story_menu_toggle_confirm_overwrite) {
        story_menu_state.confirm_overwrite = !story_menu_state.confirm_overwrite;
        g_stub_story_menu_toggle_confirm_overwrite = false;
    }
    if (g_stub_story_menu_change_state) {
        app_state = AppState::MainMenu;
        g_stub_story_menu_change_state = false;
    }
}

bool story_save_exists() {
    ++g_story_save_exists_calls;
    return g_story_save_exists_result;
}

bool load_story_career(StoryCareerData& /*out_career*/, std::string* /*error_message*/) {
    return false;
}

void reset_story_career(StoryCareerData& /*career*/) {}

void handle_story_intro_input(
    GLFWwindow* /*window*/,
    KeyEdgeState& /*edge_state*/,
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    StoryIntroState& story_intro_state,
    MatchOptions& /*options*/,
    const ControlBindings& /*controls*/,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    std::mt19937_64& /*rng*/,
    AppState& app_state,
    RuntimeAuthoredTransitionRequest& /*authored_transition_request*/,
    const KeyToNameCharFn /*key_to_name_char_fn*/,
    const TrimCopyFn /*trim_copy_fn*/,
    const StorySanitizeNameFn /*sanitize_name_fn*/,
    const StorySaveCareerCallback /*save_career_fn*/) {
    ++g_story_intro_input_calls;
    if (g_stub_story_intro_change_phase) {
        story_intro_state.phase = StoryIntroPhase::RivalIntro;
        g_stub_story_intro_change_phase = false;
    }
    if (g_stub_story_intro_toggle_accept) {
        story_intro_state.name_accept_pending = !story_intro_state.name_accept_pending;
        g_stub_story_intro_toggle_accept = false;
    }
    if (g_stub_story_intro_change_state) {
        app_state = AppState::StoryScene;
        g_stub_story_intro_change_state = false;
    }
}

bool key_to_name_char(const int /*key*/, char& /*out_char*/) {
    return false;
}

std::string trim_copy(const std::string& value) {
    return value;
}

std::string sanitize_player_name(const std::string& raw_name) {
    return raw_name;
}

bool save_story_career(const StoryCareerData& /*career_in*/, std::string* /*error_message*/) {
    ++g_story_save_career_calls;
    return true;
}

void handle_story_scene_confirm(
    StorySceneState& /*story_scene_state*/,
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    MatchOptions& /*options*/,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    std::mt19937_64& /*rng*/,
    AppState& /*app_state*/,
    RuntimeAuthoredTransitionRequest& /*authored_transition_request*/,
    const StorySaveCareerCallback /*save_career_fn*/) {
    ++g_story_scene_confirm_calls;
}

void handle_story_hub_input(
    GLFWwindow* /*window*/,
    KeyEdgeState& /*edge_state*/,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchOptions& /*options*/,
    const ControlBindings& /*controls*/,
    MatchFlowState& /*match_flow*/,
    AppState& app_state,
    whacker::sim::Simulation& /*simulation*/,
    std::mt19937_64& /*rng*/,
    const StorySaveCareerCallback /*save_career_fn*/) {
    ++g_story_hub_input_calls;
    if (g_stub_story_hub_change_row) {
        ++story_hub_state.selected_row;
        g_stub_story_hub_change_row = false;
    }
    if (g_stub_story_hub_change_week) {
        ++story_runtime.career.week;
        g_stub_story_hub_change_week = false;
    }
    if (g_stub_story_hub_change_state) {
        app_state = g_stub_story_hub_next_state;
        g_stub_story_hub_change_state = false;
    }
}

MatchExitPolicy compute_runtime_match_exit_policy(
    const whacker::sim::Simulation& /*simulation*/,
    const AppState /*app_state*/,
    const AppState /*pause_return_state*/,
    const MatchFlowState& /*match_flow*/,
    const StoryRuntimeState& /*story_runtime*/,
    const StoryIntroState& /*story_intro_state*/) {
    ++g_match_exit_policy_calls;
    return MatchExitPolicy {};
}

PauseInputFeedback handle_runtime_pause_input(
    GLFWwindow* /*window*/,
    KeyEdgeState& /*edge_state*/,
    const ControlBindings& /*controls*/,
    const MatchExitPolicy& /*exit_policy*/,
    PauseMenuState& /*pause_menu_state*/,
    AppState& /*app_state*/,
    AppState& /*pause_return_state*/,
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    StoryIntroState& /*story_intro_state*/,
    StorySceneState& /*story_scene_state*/,
    RuntimeAuthoredTransitionRequest& /*authored_transition_request*/,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    const int /*story_official_games_to_win*/,
    const StorySanitizeNameFn /*sanitize_name_fn*/,
    const StorySaveCareerCallback /*save_career_fn*/) {
    ++g_pause_input_calls;
    PauseInputFeedback feedback {};
    feedback.play_menu_move = g_stub_pause_feedback_move;
    feedback.play_menu_confirm = g_stub_pause_feedback_confirm;
    g_stub_pause_feedback_move = false;
    g_stub_pause_feedback_confirm = false;
    return feedback;
}

}  // namespace whacker::app

int main() {
    test_story_scene_pending_gate_consumes_flags_once_and_is_idempotent();
    test_story_scene_pending_gate_ignored_outside_story_scene_state();
    test_story_scene_pending_gate_runs_before_story_scene_confirm_path();
    test_escape_with_confirm_feedback_emits_menu_confirm_audio();
    test_f10_toggles_dev_info_without_menu_audio();
    test_dev_player_ai_toggle_requires_playing_surface_and_visible_dev_overlay();
    test_menu_key_transitions_between_playing_and_quick_setup_when_eligible();
    test_menu_key_does_not_open_quick_setup_during_story_match();
    test_menu_key_does_not_leave_quick_setup_when_mode_not_quick();
    test_escape_and_f10_can_coexist_in_single_tick();
    test_story_scene_confirm_suppressed_when_lockout_positive_without_pending_gate();
    test_story_scene_confirm_runs_when_lockout_zero_without_pending_gate();
    test_main_menu_and_story_hub_handlers_respect_lockout_boundary();
    test_options_menu_handler_respects_lockout_boundary_and_side_effects();
    test_story_menu_handler_respects_lockout_boundary_and_feedback();
    test_paused_handler_respects_lockout_boundary_and_feedback_audio();
    test_quick_match_setup_handler_respects_lockout_boundary_and_side_effects();
    test_story_intro_handler_respects_lockout_boundary_and_feedback();
    test_quick_match_setup_transition_to_paddle_tuning_initializes_quick_left_target();
    test_paddle_tuning_confirm_updates_quick_left_and_returns_to_quick_setup();
    test_story_hub_transition_to_paddle_tuning_initializes_story_player_target();
    test_paddle_tuning_confirm_updates_story_player_and_returns_to_story_hub();
    test_story_player_paddle_tuning_bar_adjustment_respects_skill_caps();
    test_story_player_paddle_tuning_confirm_clamps_to_skill_caps();
    test_paddle_tuning_component_selection_wraps_with_up_down();
    test_paddle_tuning_bar_adjustment_respects_budget_cap_and_noop_audio();
    test_paddle_tuning_bar_hold_repeat_applies_after_delay();
    test_paddle_tuning_hold_repeat_stops_when_both_horizontal_keys_down();
    test_main_menu_row_change_emits_move_without_confirm_audio();
    test_main_menu_state_change_emits_confirm_without_move_audio();
    test_options_audio_settings_change_emits_move_and_save_without_confirm_audio();
    test_options_binding_change_emits_confirm_and_save_without_move_audio();
    test_options_row_and_audio_change_emit_two_move_events();
    test_story_menu_confirm_selection_change_emits_move_without_confirm_audio();
    test_story_hub_row_change_emits_move_without_confirm_audio();
    test_story_hub_week_change_emits_confirm_without_move_audio();
    test_main_menu_no_mutation_emits_no_audio();
    test_options_menu_no_mutation_emits_no_audio_or_save();
    test_quick_menu_no_mutation_emits_no_audio_or_save();
    test_story_menu_no_mutation_emits_no_audio();
    test_story_intro_no_mutation_emits_no_audio();
    test_story_hub_no_mutation_emits_no_audio();
    test_paused_no_feedback_emits_no_audio();
    test_story_scene_no_confirm_input_emits_no_audio();
    test_story_scene_scroll_up_down_uses_menu_vertical_input();
    test_story_scene_confirm_snaps_to_latest_before_advancing();
    test_story_scene_binary_choice_uses_left_right_not_up_down();
    test_main_menu_combined_mutations_emit_expected_audio();
    test_options_combined_mutations_emit_expected_audio_and_side_effects();
    test_quick_menu_combined_mutations_emit_expected_audio_and_save();
    test_story_menu_combined_mutations_emit_expected_audio();
    test_story_hub_combined_mutations_emit_expected_audio();
    test_story_intro_combined_mutations_emit_single_confirm_audio();
    test_paused_combined_feedback_emits_move_and_confirm_audio();
    test_story_menu_save_exists_resolution_respects_lockout_and_cache();
    return 0;
}
