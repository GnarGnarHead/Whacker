#pragma once

#ifdef WHACKER_HAS_GLFW

#include "match_exit_policy.hpp"
#include "match_flow.hpp"
#include "menu_input.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

struct GLFWwindow;

namespace whacker::app {

struct PauseInputFeedback {
    bool play_menu_move = false;
    bool play_menu_confirm = false;
};

PauseInputFeedback handle_runtime_pause_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    const ControlBindings& controls,
    const MatchExitPolicy& exit_policy,
    PauseMenuState& pause_menu_state,
    AppState& app_state,
    AppState& pause_return_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    StorySceneState& story_scene_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    int story_official_games_to_win,
    StorySanitizeNameFn sanitize_name_fn,
    StorySaveCareerCallback save_career_fn);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
