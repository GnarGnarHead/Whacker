#pragma once

#ifdef WHACKER_HAS_GLFW

#include "match_exit_policy.hpp"
#include "match_flow.hpp"
#include "menu_overlay.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

MatchExitPolicy compute_runtime_match_exit_policy(
    const whacker::sim::Simulation& simulation,
    AppState app_state,
    AppState pause_return_state,
    const MatchFlowState& match_flow,
    const StoryRuntimeState& story_runtime,
    const StoryIntroState& story_intro_state);

void execute_runtime_pause_exit(
    const MatchExitPolicy& policy,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    StorySceneState& story_scene_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    AppState& app_state,
    int story_official_games_to_win,
    StorySanitizeNameFn sanitize_name_fn,
    StorySaveCareerCallback save_career_fn);

void quit_runtime_to_main_menu(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    StorySceneState& story_scene_state,
    MatchFlowState& match_flow,
    PauseMenuState& pause_menu_state,
    AppState& pause_return_state,
    whacker::sim::Simulation& simulation,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    int story_official_games_to_win,
    StorySaveCareerCallback save_career_fn,
    AppState& app_state);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
