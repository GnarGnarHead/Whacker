#pragma once

#include "match_exit_policy.hpp"
#include "match_flow.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "ui_state.hpp"

namespace whacker::app {

MatchExitPolicy compute_runtime_match_exit_policy(
    const whacker::sim::Simulation& simulation,
    AppState app_state,
    AppState pause_return_state,
    const MatchFlowState& match_flow,
    const StoryRuntimeState& story_runtime,
    const StoryIntroState& story_intro_state);

}  // namespace whacker::app
