#pragma once

#include <string>

#include "app_types.hpp"
#include "match_flow.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

using StoryResetCareerFn = void (*)(StoryCareerData&);
using StorySaveCareerCallback = bool (*)(const StoryCareerData&, std::string*);
using StorySanitizeNameFn = std::string (*)(const std::string&);

bool story_hub_row_enabled(StoryHubRow row, const StoryCareerData& career);
void copy_onboarding_runtime_to_career(StoryRuntimeState& story_runtime);
void copy_onboarding_career_to_runtime(StoryRuntimeState& story_runtime);

void begin_new_story_intro(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchOptions& options,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    AppState& app_state,
    StoryResetCareerFn reset_career_fn = nullptr);

void complete_story_intro(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    AppState& app_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    StorySanitizeNameFn sanitize_name_fn = nullptr,
    StorySaveCareerCallback save_career_fn = nullptr);

}  // namespace whacker::app
