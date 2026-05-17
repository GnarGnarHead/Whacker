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

enum class RuntimeStepBranchOutcome {
    NeedsCommonSimulationStep,
    SteppedInsideBranch,
};

struct RuntimeStepPhaseContext {
    GLFWwindow* window;
    AppState& app_state;
    const bool ai_controls_player_paddle;
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
    int story_official_games_to_win;
};

RuntimeStepBranchOutcome step_story_intro(
    RuntimeStepPhaseContext& context,
    const RuntimeStepInputSnapshot& step_input,
    double now,
    float& type_blip_cooldown,
    std::uint32_t& type_blip_pattern_step);
RuntimeStepBranchOutcome step_story_scene(
    RuntimeStepPhaseContext& context,
    const RuntimeStepInputSnapshot& step_input,
    double now,
    float& type_blip_cooldown,
    std::uint32_t& type_blip_pattern_step);
RuntimeStepBranchOutcome step_non_playing_ambient(RuntimeStepPhaseContext& context);
RuntimeStepBranchOutcome step_playing(RuntimeStepPhaseContext& context);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
