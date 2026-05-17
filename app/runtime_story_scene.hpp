#pragma once

#ifdef WHACKER_HAS_GLFW

#include <random>

#include "app_types.hpp"
#include "match_flow.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "story_state.hpp"

namespace whacker::app {

void handle_story_scene_confirm(
    StorySceneState& story_scene_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchOptions& options,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    AppState& app_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    StorySaveCareerCallback save_career_fn);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
