#include "test_assert.hpp"
#include <cmath>
#include <cstdint>
#include <random>
#include <string>

#include <GLFW/glfw3.h>

#include "match_end_flow.hpp"
#include "match_flow.hpp"
#include "menu_input.hpp"
#include "play_control.hpp"
#include "runtime_helpers.hpp"
#include "runtime_step_phase.hpp"
#include "story_intro.hpp"
#include "story_match.hpp"
#include "story_save.hpp"
#include "story_scene.hpp"
#include "story_script_catalog.hpp"

namespace {

struct StubState {
    int update_targets_for_play_calls = 0;
    int update_targets_for_ambient_calls = 0;
    int end_active_or_quick_match_calls = 0;
    int audio_event_calls = 0;
    bool last_update_targets_received_overrides = false;
    whacker::app::PlayControlOverrides last_update_targets_overrides {};

    void reset() {
        *this = StubState {};
    }
};

StubState g_stub_state {};
int& g_update_targets_for_play_calls = g_stub_state.update_targets_for_play_calls;
int& g_update_targets_for_ambient_calls = g_stub_state.update_targets_for_ambient_calls;
int& g_end_active_or_quick_match_calls = g_stub_state.end_active_or_quick_match_calls;
int& g_audio_event_calls = g_stub_state.audio_event_calls;
bool& g_last_update_targets_received_overrides = g_stub_state.last_update_targets_received_overrides;
whacker::app::PlayControlOverrides& g_last_update_targets_overrides = g_stub_state.last_update_targets_overrides;

constexpr float kAmbientLeftTarget = 120.0f;
constexpr float kAmbientRightTarget = 220.0f;

void reset_stubs() {
    g_stub_state.reset();
}

void test_paused_step_consumes_accumulator_without_advancing_simulation() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 42.0;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt) * 3.0;
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::Paused;
    const bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    auto& state = simulation.mutable_state();
    state.left_score = 4;
    state.right_score = 7;
    const whacker::sim::RallyState before = simulation.state();
    static_cast<void>(before);
    std::mt19937_64 rng {0x11112222ULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.20f;
    std::uint32_t type_blip_pattern_step = 5u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    constexpr double kFixedDtSeconds = static_cast<double>(whacker::sim::kFixedDt);
    TEST_CHECK(std::abs(accumulator) < 1.0e-9);
    TEST_CHECK(app_state == whacker::app::AppState::Paused);
    TEST_CHECK(simulation.state().left_score == before.left_score);
    TEST_CHECK(simulation.state().right_score == before.right_score);
    TEST_CHECK(simulation.state().rally_hits == before.rally_hits);
    const float expected_cooldown = 0.20f - static_cast<float>(3.0 * kFixedDtSeconds);
    static_cast<void>(expected_cooldown);
    TEST_CHECK(std::fabs(type_blip_cooldown - expected_cooldown) < 1.0e-6f);
    TEST_CHECK(type_blip_pattern_step == 5u);
    TEST_CHECK(g_update_targets_for_play_calls == 0);
    TEST_CHECK(g_update_targets_for_ambient_calls == 0);
    TEST_CHECK(g_end_active_or_quick_match_calls == 0);
    TEST_CHECK(g_audio_event_calls == 0);
}

void test_non_playing_step_uses_ambient_ai_targets_without_play_update() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 99.0;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::MainMenu;
    const bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng {0x33334444ULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    const auto& after = simulation.state();
    static_cast<void>(after);
    TEST_CHECK(std::abs(accumulator) < 1.0e-9);
    TEST_CHECK(app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(g_update_targets_for_play_calls == 0);
    TEST_CHECK(g_update_targets_for_ambient_calls == 1);
    TEST_CHECK(g_end_active_or_quick_match_calls == 0);
    TEST_CHECK(std::fabs(after.left.target_y - kAmbientLeftTarget) < 1.0e-5f);
    TEST_CHECK(std::fabs(after.right.target_y - kAmbientRightTarget) < 1.0e-5f);
    TEST_CHECK(std::fabs(after.left.feedforward_velocity_y) < 1.0e-6f);
    TEST_CHECK(std::fabs(after.right.feedforward_velocity_y) < 1.0e-6f);
}

void test_story_intro_play_match_uses_overrides_without_mutating_options() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 17.0;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::StoryIntro;
    const bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    options.left_mode = whacker::app::PaddleMode::Human;
    options.right_mode = whacker::app::PaddleMode::Human;
    options.left_ai_style = whacker::app::AiStyle::Spin;
    options.right_ai_style = whacker::app::AiStyle::Maxed;
    const whacker::app::MatchOptions options_before = options;
    static_cast<void>(options_before);
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.career.player_skills = {.edge = 0.31f, .power = 0.27f, .spin_inject = 0.19f};
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    story_intro_state.phase = whacker::app::StoryIntroPhase::PlayMatch;
    story_intro_state.player_is_right = true;
    story_intro_state.rival_skills = {.edge = 0.14f, .power = 0.18f, .spin_inject = 0.12f};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng {0x71727374ULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(g_update_targets_for_play_calls == 1);
    TEST_CHECK(g_last_update_targets_received_overrides);
    TEST_CHECK(g_last_update_targets_overrides.force_modes);
    TEST_CHECK(g_last_update_targets_overrides.left_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(g_last_update_targets_overrides.right_mode == whacker::app::PaddleMode::Human);
    TEST_CHECK(!g_last_update_targets_overrides.ai_training_context);
    TEST_CHECK(g_last_update_targets_overrides.override_left_skills);
    TEST_CHECK(g_last_update_targets_overrides.override_right_skills);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.edge == story_intro_state.rival_skills.edge);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.power == story_intro_state.rival_skills.power);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.spin_inject == story_intro_state.rival_skills.spin_inject);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.edge == story_runtime.career.player_skills.edge);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.power == story_runtime.career.player_skills.power);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.spin_inject == story_runtime.career.player_skills.spin_inject);
    TEST_CHECK(options.left_mode == options_before.left_mode);
    TEST_CHECK(options.right_mode == options_before.right_mode);
    TEST_CHECK(options.left_ai_style == options_before.left_ai_style);
    TEST_CHECK(options.right_ai_style == options_before.right_ai_style);
}

void test_story_intro_play_match_toggle_forces_both_ai() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 18.0;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::StoryIntro;
    const bool ai_controls_player_paddle = true;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    story_intro_state.phase = whacker::app::StoryIntroPhase::PlayMatch;
    story_intro_state.player_is_right = false;
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng {0x81828384ULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(g_update_targets_for_play_calls == 1);
    TEST_CHECK(g_last_update_targets_received_overrides);
    TEST_CHECK(g_last_update_targets_overrides.force_modes);
    TEST_CHECK(g_last_update_targets_overrides.left_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(g_last_update_targets_overrides.right_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(!story_runtime.imagination_takeover_cue_shown);
    TEST_CHECK(story_runtime.imagination_takeover_cue_seconds == 0.0f);
}

void test_playing_story_match_override_uses_story_player_side_when_dev_ai_disabled() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 21.0;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;
    const bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.career.player_skills = {.edge = 0.34f, .power = 0.21f, .spin_inject = 0.17f};
    story_runtime.active_match = whacker::app::StoryMatchKind::Official;
    story_runtime.active_rival_id = whacker::app::StoryRivalId::Rook;
    story_runtime.active_rival_skills = {.edge = 0.22f, .power = 0.39f, .spin_inject = 0.25f};
    story_runtime.career.prefers_right_side = false;
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng {0x44445555ULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(g_update_targets_for_play_calls == 1);
    TEST_CHECK(g_last_update_targets_received_overrides);
    TEST_CHECK(g_last_update_targets_overrides.force_modes);
    TEST_CHECK(g_last_update_targets_overrides.left_mode == whacker::app::PaddleMode::Human);
    TEST_CHECK(g_last_update_targets_overrides.right_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(!g_last_update_targets_overrides.ai_training_context);
    TEST_CHECK(g_last_update_targets_overrides.override_left_skills);
    TEST_CHECK(g_last_update_targets_overrides.override_right_skills);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.edge == story_runtime.career.player_skills.edge);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.power == story_runtime.career.player_skills.power);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.spin_inject == story_runtime.career.player_skills.spin_inject);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.edge == story_runtime.active_rival_skills.edge);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.power == story_runtime.active_rival_skills.power);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.spin_inject == story_runtime.active_rival_skills.spin_inject);
}

void test_playing_story_match_override_forces_both_ai_when_dev_ai_enabled() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 22.0;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;
    const bool ai_controls_player_paddle = true;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::Official;
    story_runtime.career.prefers_right_side = false;
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng {0x55556666ULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(g_update_targets_for_play_calls == 1);
    TEST_CHECK(g_last_update_targets_received_overrides);
    TEST_CHECK(g_last_update_targets_overrides.force_modes);
    TEST_CHECK(g_last_update_targets_overrides.left_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(g_last_update_targets_overrides.right_mode == whacker::app::PaddleMode::AI);
}

void test_imagination_match_keeps_both_paddles_ai_before_preview_points() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 22.25;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;
    const bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::Imagination1967;
    story_runtime.career.prefers_right_side = false;
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    {
        auto& state = simulation.mutable_state();
        state.left_score = 2;
        state.right_score = 1;
    }
    std::mt19937_64 rng {0x595A5B5CULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(g_update_targets_for_play_calls == 1);
    TEST_CHECK(g_last_update_targets_received_overrides);
    TEST_CHECK(g_last_update_targets_overrides.force_modes);
    TEST_CHECK(g_last_update_targets_overrides.left_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(g_last_update_targets_overrides.right_mode == whacker::app::PaddleMode::AI);
}

void test_imagination_match_handoffs_to_player_after_preview_points() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 22.75;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;
    const bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.career.player_skills = {.edge = 0.11f, .power = 0.22f, .spin_inject = 0.33f};
    story_runtime.active_match = whacker::app::StoryMatchKind::Imagination1967;
    story_runtime.career.prefers_right_side = false;
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    {
        auto& state = simulation.mutable_state();
        state.left_score = 2;
        state.right_score = 2;
    }
    std::mt19937_64 rng {0x5A5B5C5DULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(g_update_targets_for_play_calls == 1);
    TEST_CHECK(g_last_update_targets_received_overrides);
    TEST_CHECK(g_last_update_targets_overrides.force_modes);
    TEST_CHECK(g_last_update_targets_overrides.left_mode == whacker::app::PaddleMode::Human);
    TEST_CHECK(g_last_update_targets_overrides.right_mode == whacker::app::PaddleMode::AI);
    const whacker::progression::SkillState champion_player =
        whacker::app::story_script_imagination_1967_player_skills();
    const whacker::progression::SkillState champion_rival =
        whacker::app::story_script_imagination_1967_rival_skills();
    TEST_CHECK(g_last_update_targets_overrides.left_skills.edge == champion_player.edge);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.power == champion_player.power);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.spin_inject == champion_player.spin_inject);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.edge == champion_rival.edge);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.power == champion_rival.power);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.spin_inject == champion_rival.spin_inject);
    TEST_CHECK(story_runtime.imagination_takeover_cue_shown);
    TEST_CHECK(story_runtime.imagination_takeover_cue_seconds > 0.0f);
}

void test_imagination_match_handoffs_player_right_with_player_champion_profile() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 23.05;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;
    const bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.career.player_skills = {.edge = 0.19f, .power = 0.21f, .spin_inject = 0.14f};
    story_runtime.active_match = whacker::app::StoryMatchKind::Imagination1967;
    story_runtime.career.prefers_right_side = true;
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    {
        auto& state = simulation.mutable_state();
        state.left_score = 2;
        state.right_score = 2;
    }
    std::mt19937_64 rng {0x5D5E5F60ULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(g_update_targets_for_play_calls == 1);
    TEST_CHECK(g_last_update_targets_received_overrides);
    TEST_CHECK(g_last_update_targets_overrides.force_modes);
    TEST_CHECK(g_last_update_targets_overrides.left_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(g_last_update_targets_overrides.right_mode == whacker::app::PaddleMode::Human);
    const whacker::progression::SkillState champion_player =
        whacker::app::story_script_imagination_1967_player_skills();
    const whacker::progression::SkillState champion_rival =
        whacker::app::story_script_imagination_1967_rival_skills();
    TEST_CHECK(g_last_update_targets_overrides.right_skills.edge == champion_player.edge);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.power == champion_player.power);
    TEST_CHECK(g_last_update_targets_overrides.right_skills.spin_inject == champion_player.spin_inject);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.edge == champion_rival.edge);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.power == champion_rival.power);
    TEST_CHECK(g_last_update_targets_overrides.left_skills.spin_inject == champion_rival.spin_inject);
}

void test_imagination_takeover_cue_emits_once_and_counts_down() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 22.95;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;
    const bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::Imagination1967;
    story_runtime.career.prefers_right_side = true;
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    {
        auto& state = simulation.mutable_state();
        state.left_score = 2;
        state.right_score = 2;
    }
    std::mt19937_64 rng {0x5C5D5E5FULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    const float first_seconds = story_runtime.imagination_takeover_cue_seconds;
    TEST_CHECK(story_runtime.imagination_takeover_cue_shown);
    TEST_CHECK(first_seconds > 1.0f);

    accumulator = static_cast<double>(whacker::sim::kFixedDt);
    whacker::app::handle_runtime_step_phase(
        window,
        now + static_cast<double>(whacker::sim::kFixedDt),
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(story_runtime.imagination_takeover_cue_shown);
    TEST_CHECK(story_runtime.imagination_takeover_cue_seconds < first_seconds);
}

void test_playing_training_match_override_sets_training_context() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 22.5;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;
    const bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::Training;
    story_runtime.career.prefers_right_side = true;
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng {0x56577889ULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(g_update_targets_for_play_calls == 1);
    TEST_CHECK(g_last_update_targets_received_overrides);
    TEST_CHECK(g_last_update_targets_overrides.force_modes);
    TEST_CHECK(g_last_update_targets_overrides.ai_training_context);
}

void test_playing_quick_match_override_converts_human_modes_to_ai_when_dev_ai_enabled() {
    reset_stubs();

    GLFWwindow* window = nullptr;
    const double now = 23.0;
    double accumulator = static_cast<double>(whacker::sim::kFixedDt);
    const whacker::app::RuntimeStepInputSnapshot step_input {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;
    const bool ai_controls_player_paddle = true;
    whacker::app::MatchOptions options {};
    options.left_mode = whacker::app::PaddleMode::Human;
    options.right_mode = whacker::app::PaddleMode::Human;
    whacker::app::ControlBindings controls {};
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::None;
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng {0x66667777ULL};
    whacker::app::AudioEngine audio_engine {};
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;

    whacker::app::handle_runtime_step_phase(
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        3);

    TEST_CHECK(g_update_targets_for_play_calls == 1);
    TEST_CHECK(g_last_update_targets_received_overrides);
    TEST_CHECK(g_last_update_targets_overrides.force_modes);
    TEST_CHECK(g_last_update_targets_overrides.left_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(g_last_update_targets_overrides.right_mode == whacker::app::PaddleMode::AI);
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

void AudioEngine::set_settings(const AudioSettings& /*settings*/) {}

AudioSettings AudioEngine::settings() const {
    return AudioSettings {};
}

void AudioEngine::push_event(const AudioEventId /*event_id*/) {
    ++::g_audio_event_calls;
}

void AudioEngine::push_paddle_hit(const PaddleHitAudioParams& /*params*/) {}

void AudioEngine::push_wall_hit(const WallHitAudioParams& /*params*/) {}

AudioSettings clamp_audio_settings(const AudioSettings settings) {
    return settings;
}

void end_active_or_quick_match(
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    StorySceneState& /*story_scene_state*/,
    RuntimeAuthoredTransitionRequest& /*authored_transition_request*/,
    AppState& /*app_state*/,
    StoryMatchEndReason /*end_reason*/,
    const int /*story_official_games_to_win*/,
    StorySaveCareerCallback /*save_career_fn*/) {
    ++::g_end_active_or_quick_match_calls;
}

void update_targets_for_play(
    GLFWwindow* /*window*/,
    whacker::sim::Simulation& /*simulation*/,
    const MatchOptions& /*options*/,
    const ControlBindings& /*controls*/,
    RuntimeAiState& /*left_ai_state*/,
    RuntimeAiState& /*right_ai_state*/,
    const float /*dt*/,
    const PlayControlOverrides* overrides) {
    ++::g_update_targets_for_play_calls;
    ::g_last_update_targets_received_overrides = (overrides != nullptr);
    if (overrides != nullptr) {
        ::g_last_update_targets_overrides = *overrides;
    } else {
        ::g_last_update_targets_overrides = PlayControlOverrides {};
    }
}

void update_targets_for_ambient(
    whacker::sim::Simulation& simulation,
    const MatchOptions& /*options*/,
    RuntimeAiState& /*left_ai_state*/,
    RuntimeAiState& /*right_ai_state*/,
    const float /*dt*/) {
    ++::g_update_targets_for_ambient_calls;
    auto& state = simulation.mutable_state();
    state.left.target_y = ::kAmbientLeftTarget;
    state.right.target_y = ::kAmbientRightTarget;
    state.left.feedforward_velocity_y = 0.0f;
    state.right.feedforward_velocity_y = 0.0f;
}

void set_paddle_execution_full(whacker::sim::PaddleState& /*paddle*/) {}

void set_paddle_execution_from_skills(
    whacker::sim::PaddleState& /*paddle*/,
    const whacker::progression::SkillState& /*skills*/) {}

void sync_runtime_ai_style(RuntimeAiState& /*ai_state*/, const AiStyle /*style*/) {}

void track_intro_contact_usage(
    StoryIntroState& /*story_intro_state*/,
    const whacker::sim::SimulationConfig& /*config*/,
    const whacker::sim::RallyState& /*before*/,
    const whacker::sim::RallyState& /*after*/) {}

bool detect_wall_bounce(
    const whacker::sim::RallyState& /*before*/,
    const whacker::sim::RallyState& /*after*/,
    const whacker::sim::SimulationConfig& /*config*/) {
    return false;
}

PaddleHitAudioParams build_paddle_hit_audio_params(
    const whacker::sim::RallyState& /*before*/,
    const whacker::sim::RallyState& /*after*/,
    const whacker::sim::SimulationConfig& /*config*/) {
    return PaddleHitAudioParams {};
}

WallHitAudioParams build_wall_hit_audio_params(
    const whacker::sim::RallyState& /*before*/,
    const whacker::sim::RallyState& /*after*/,
    const whacker::sim::SimulationConfig& /*config*/) {
    return WallHitAudioParams {};
}

bool table_tennis_game_complete(const int /*left_score*/, const int /*right_score*/, int* winner_out) {
    if (winner_out != nullptr) {
        *winner_out = 0;
    }
    return false;
}

void update_serve_after_scored_point(
    MatchFlowState& /*match_flow*/,
    const whacker::sim::RallyState& /*score_state*/,
    whacker::sim::Simulation& /*simulation*/) {}

void start_next_table_tennis_game(
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    const bool /*alternate_opener*/) {}

void reset_match_flow(MatchFlowState& /*match_flow*/) {}

bool update_match_opening_countdown(
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    const float /*dt*/) {
    return false;
}

bool match_opening_ball_visible(const MatchFlowState& /*match_flow*/) {
    return false;
}

void update_story_intro_typewriter(
    StoryIntroState& /*story_intro_state*/,
    const ControlBindings& /*controls*/,
    const float /*dt*/,
    const float /*speed_multiplier*/,
    StoryIntroKeyNameFn /*key_name_fn*/,
    StoryIntroSanitizeNameFn /*sanitize_name_fn*/) {}

void reset_story_intro_typewriter(StoryIntroState& /*story_intro_state*/) {}

void update_story_scene_typewriter(StorySceneState& /*scene_state*/, const float /*dt*/, const float /*speed_multiplier*/) {
}

StorySceneSpeaker story_scene_current_speaker(const StorySceneState& /*scene_state*/) {
    return StorySceneSpeaker::None;
}

const char* key_name(const int /*key*/) {
    return "";
}

void clear_last_pressed_key() {}

std::string sanitize_player_name(const std::string& raw_name) {
    return raw_name;
}

bool save_story_career(const StoryCareerData& /*career_in*/, std::string* /*error_message*/) {
    return true;
}

void update_story_match_tracking(
    StoryRuntimeState& /*story_runtime*/,
    const whacker::sim::SimulationConfig& /*config*/,
    const whacker::sim::RallyState& /*before*/,
    const whacker::sim::RallyState& /*after*/,
    const float /*dt*/) {}

}  // namespace whacker::app

int main() {
    test_paused_step_consumes_accumulator_without_advancing_simulation();
    test_non_playing_step_uses_ambient_ai_targets_without_play_update();
    test_story_intro_play_match_uses_overrides_without_mutating_options();
    test_story_intro_play_match_toggle_forces_both_ai();
    test_playing_story_match_override_uses_story_player_side_when_dev_ai_disabled();
    test_playing_story_match_override_forces_both_ai_when_dev_ai_enabled();
    test_imagination_match_keeps_both_paddles_ai_before_preview_points();
    test_imagination_match_handoffs_to_player_after_preview_points();
    test_imagination_match_handoffs_player_right_with_player_champion_profile();
    test_imagination_takeover_cue_emits_once_and_counts_down();
    test_playing_training_match_override_sets_training_context();
    test_playing_quick_match_override_converts_human_modes_to_ai_when_dev_ai_enabled();
    return 0;
}
