#pragma once

#include "sim/config.hpp"
#include "sim/types.hpp"

namespace whacker::sim {

bool handle_wall_bounce(BallState& ball, const SimulationConfig& config);
ScoreEvent handle_scoring(const BallState& ball, const SimulationConfig& config);
bool handle_paddle_collision(
    BallState& ball,
    const Vec2& previous_position,
    const PaddleState& paddle,
    const SimulationConfig& config,
    bool is_left_paddle,
    float* contact_u_out = nullptr,
    float technical_scale = 1.0f,
    float spin_scale = 1.0f);

}  // namespace whacker::sim
