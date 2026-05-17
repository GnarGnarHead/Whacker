#pragma once

#include "sim/physics.hpp"

namespace whacker::app {

void set_human_target(
    whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config,
    bool move_up,
    bool move_down,
    float dt);

}  // namespace whacker::app

