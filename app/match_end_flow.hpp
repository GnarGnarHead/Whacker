#pragma once

#include "match_flow.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_match.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

void end_active_or_quick_match(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    StorySceneState& story_scene_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    AppState& app_state,
    StoryMatchEndReason end_reason,
    int story_official_games_to_win,
    StorySaveCareerCallback save_career_fn = nullptr);

}  // namespace whacker::app
