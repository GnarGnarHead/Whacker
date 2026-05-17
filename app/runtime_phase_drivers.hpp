#pragma once

#ifdef WHACKER_HAS_GLFW

#include <cstdint>
#include <random>

#include "app_types.hpp"
#include "audio_engine.hpp"
#include "match_flow.hpp"
#include "menu_input.hpp"
#include "paddle_tuning.hpp"
#include "runtime_story_save_cache.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

struct GLFWwindow;

namespace whacker::app {

struct RuntimeUpdatePhaseContext {
    GLFWwindow* window;
    KeyEdgeState& edge_state;
    AppState& app_state;
    AppState& pause_return_state;
    bool& show_dev_info;
    bool& ai_controls_player_paddle;
    MatchOptions& options;
    ControlBindings& controls;
    AudioSettings& audio_settings;
    AudioEngine& audio_engine;
    MainMenuState& main_menu_state;
    OptionsMenuState& options_menu_state;
    PauseMenuState& pause_menu_state;
    MenuState& menu_state;
    PaddleTuningState& paddle_tuning_state;
    StoryMenuState& story_menu_state;
    StoryIntroState& story_intro_state;
    StorySceneState& story_scene_state;
    StoryHubState& story_hub_state;
    StoryRuntimeState& story_runtime;
    RuntimeVisualTransitionState& visual_transition;
    RuntimeAuthoredTransitionRequest& authored_transition_request;
    MatchFlowState& match_flow;
    RuntimeAiState& left_ai_state;
    RuntimeAiState& right_ai_state;
    whacker::sim::Simulation& simulation;
    std::mt19937_64& rng;
    float& type_blip_cooldown;
    std::uint32_t& type_blip_pattern_step;
};

void run_runtime_update_phases(
    RuntimeUpdatePhaseContext& context,
    double now,
    double& accumulator,
    double& menu_input_lockout,
    double menu_input_lockout_seconds,
    int story_official_games_to_win,
    RuntimeStorySaveExistsCache* story_save_cache = nullptr);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
