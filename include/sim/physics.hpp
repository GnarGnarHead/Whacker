#pragma once

#include "sim/config.hpp"
#include "sim/types.hpp"

namespace whacker::sim {

class Simulation {
public:
    explicit Simulation(SimulationConfig config = {});

    void reset();
    ScoreEvent step(float dt = kFixedDt);

    const SimulationConfig& config() const;
    const RallyState& state() const;
    RallyState& mutable_state();

private:
    SimulationConfig config_ {};
    RallyState state_ {};

    void integrate_paddle(PaddleState& paddle, float dt);
    void reset_ball(bool serve_to_right);
};

}  // namespace whacker::sim

