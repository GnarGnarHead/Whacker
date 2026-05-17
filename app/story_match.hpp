#pragma once

#include <cstdint>
#include <random>
#include <string>

#include "match_flow.hpp"
#include "sim/physics.hpp"
#include "story_state.hpp"

namespace whacker::app {

using StorySaveCareerFn = bool (*)(const StoryCareerData&, std::string*);

enum class StoryMatchEndReason : std::uint8_t {
    Completed = 0,
    Forfeit = 1,
    EndTraining = 2
};

void reset_story_match_tracking(StoryRuntimeState& story_runtime);

void start_story_match(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchOptions& options,
    whacker::sim::Simulation& simulation,
    MatchFlowState& match_flow,
    std::mt19937_64& rng,
    StoryMatchKind match_kind);

void update_story_match_tracking(
    StoryRuntimeState& story_runtime,
    const whacker::sim::SimulationConfig& config,
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    float dt);

void finalize_story_match(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    const whacker::sim::RallyState& final_state,
    StorySaveCareerFn save_career_fn = nullptr,
    StoryMatchEndReason end_reason = StoryMatchEndReason::Completed);

void advance_story_week(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StorySaveCareerFn save_career_fn = nullptr);

}  // namespace whacker::app
