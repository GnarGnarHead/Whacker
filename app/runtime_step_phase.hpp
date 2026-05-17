#pragma once

#ifdef WHACKER_HAS_GLFW

#include <cstdint>
#include <random>

#include "app_types.hpp"
#include "audio_engine.hpp"
#include "match_flow.hpp"
#include "runtime_step_input.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"

struct GLFWwindow;

namespace whacker::app {

struct RuntimeStepPhaseArgs {
    GLFWwindow* window;
    double now;
    double& accumulator;
    const RuntimeStepInputSnapshot& step_input;
    AppState& app_state;
    bool ai_controls_player_paddle;
    MatchOptions& options;
    const ControlBindings& controls;
    StoryRuntimeState& story_runtime;
    StoryHubState& story_hub_state;
    StoryIntroState& story_intro_state;
    StorySceneState& story_scene_state;
    RuntimeAuthoredTransitionRequest& authored_transition_request;
    MatchFlowState& match_flow;
    RuntimeAiState& left_ai_state;
    RuntimeAiState& right_ai_state;
    whacker::sim::Simulation& simulation;
    std::mt19937_64& rng;
    AudioEngine& audio_engine;
    float& type_blip_cooldown;
    std::uint32_t& type_blip_pattern_step;
    int story_official_games_to_win;
};

void handle_runtime_step_phase(RuntimeStepPhaseArgs& args);

void handle_runtime_step_phase(
    GLFWwindow* window,
    double now,
    double& accumulator,
    const RuntimeStepInputSnapshot& step_input,
    AppState& app_state,
    bool ai_controls_player_paddle,
    MatchOptions& options,
    const ControlBindings& controls,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    StorySceneState& story_scene_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    MatchFlowState& match_flow,
    RuntimeAiState& left_ai_state,
    RuntimeAiState& right_ai_state,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    AudioEngine& audio_engine,
    float& type_blip_cooldown,
    std::uint32_t& type_blip_pattern_step,
    int story_official_games_to_win);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
