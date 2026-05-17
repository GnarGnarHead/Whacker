#pragma once

#include <random>

#include "app_types.hpp"
#include "match_flow.hpp"
#include "progression/skills.hpp"
#include "sim/physics.hpp"

namespace whacker::app {

void start_story_play_session(
    MatchOptions& options,
    whacker::sim::Simulation& simulation,
    MatchFlowState& match_flow,
    std::mt19937_64& rng,
    ActiveMatchMode mode,
    bool player_is_right,
    AiStyle rival_style,
    const whacker::progression::SkillState& rival_skills,
    const whacker::progression::SkillState& player_skills);

}  // namespace whacker::app
