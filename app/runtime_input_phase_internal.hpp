#pragma once

#ifdef WHACKER_HAS_GLFW

#include <random>

#include "app_types.hpp"
#include "audio_engine.hpp"
#include "match_flow.hpp"
#include "menu_input.hpp"
#include "paddle_tuning.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

struct GLFWwindow;

namespace whacker::app {

struct RuntimeInputBranchEffects {
    int menu_move_events = 0;
    int menu_confirm_events = 0;
    bool persist_menu_settings = false;
};

struct RuntimeInputPhaseContext {
    GLFWwindow* window;
    KeyEdgeState& edge_state;
    AppState& app_state;
    AppState& pause_return_state;
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
    RuntimeAuthoredTransitionRequest& authored_transition_request;
    MatchFlowState& match_flow;
    whacker::sim::Simulation& simulation;
    std::mt19937_64& rng;
    int story_official_games_to_win;
};

void apply_runtime_input_branch_effects(const RuntimeInputBranchEffects& effects, RuntimeInputPhaseContext& context);
void handle_main_menu_branch(RuntimeInputPhaseContext& context);
void handle_options_menu_branch(RuntimeInputPhaseContext& context);
void handle_quick_match_setup_branch(RuntimeInputPhaseContext& context);
void handle_story_menu_branch(RuntimeInputPhaseContext& context, bool has_save);
void handle_story_intro_branch(RuntimeInputPhaseContext& context);
void handle_story_scene_branch(RuntimeInputPhaseContext& context);
void handle_story_hub_branch(RuntimeInputPhaseContext& context);
void handle_paddle_tuning_branch(RuntimeInputPhaseContext& context);
void handle_paused_branch(RuntimeInputPhaseContext& context);
void handle_runtime_global_input(RuntimeInputPhaseContext& context, bool& show_dev_info);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
