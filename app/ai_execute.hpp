#pragma once

#include "ai_planner_internal.hpp"

namespace whacker::app::ai_internal {

void apply_ai_decision_to_paddle(
    whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config,
    const AiDecision& decision,
    bool inbound,
    float intercept_time_s,
    bool ambient_mode);

}  // namespace whacker::app::ai_internal
