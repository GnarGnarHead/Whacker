#pragma once

#include "sim/config.hpp"
#include "sim/types.hpp"

namespace whacker::sim {

void apply_spin_curve(BallState& ball, const SimulationConfig& config, float dt);
void decay_spin(BallState& ball, const SimulationConfig& config, float dt);
void decay_speed_scalar(BallState& ball, const SimulationConfig& config, float dt);
void inject_spin(
    BallState& ball,
    const SimulationConfig& config,
    float paddle_velocity,
    float shot_direction_x,
    float spin_scale = 1.0f);

}  // namespace whacker::sim
