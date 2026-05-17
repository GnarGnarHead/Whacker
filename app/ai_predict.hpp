#pragma once

#include "ai_planner_internal.hpp"

namespace whacker::app::ai_internal {

bool moving_toward_actor_paddle(const whacker::sim::RallyState& state);
float actor_paddle_contact_plane_x(const whacker::sim::SimulationConfig& config);
PredictorResult predict_intercept(
    const whacker::sim::RallyState& state,
    const whacker::sim::SimulationConfig& config,
    int max_steps);

}  // namespace whacker::app::ai_internal
