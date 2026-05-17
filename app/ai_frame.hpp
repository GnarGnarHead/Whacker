#pragma once

#include "ai_core.hpp"

namespace whacker::app::ai_internal {

whacker::sim::RallyState mirror_rally_state_x(
    const whacker::sim::SimulationConfig& config,
    const whacker::sim::RallyState& source);

whacker::sim::Simulation make_actor_frame_simulation(
    const whacker::sim::Simulation& simulation,
    bool for_left_paddle);

AiDecision actor_decision_to_world(const AiDecision& actor_decision, bool for_left_paddle);

}  // namespace whacker::app::ai_internal
