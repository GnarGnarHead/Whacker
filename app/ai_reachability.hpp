#pragma once

#include "ai_planner_internal.hpp"

namespace whacker::app::ai_internal {

ReachabilityEnvelope compute_reachability_envelope(
    const whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config,
    float horizon_s,
    int max_steps,
    float speed_scale,
    float accel_scale);

}  // namespace whacker::app::ai_internal
