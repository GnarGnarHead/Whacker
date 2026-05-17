#include "ai_frame.hpp"

namespace whacker::app::ai_internal {

whacker::sim::RallyState mirror_rally_state_x(
    const whacker::sim::SimulationConfig& config,
    const whacker::sim::RallyState& source) {
    whacker::sim::RallyState mirrored = source;
    mirrored.ball.position.x = config.court_width - source.ball.position.x;
    mirrored.ball.position.y = source.ball.position.y;
    mirrored.ball.velocity.x = -source.ball.velocity.x;
    mirrored.ball.velocity.y = source.ball.velocity.y;
    mirrored.ball.spin = -source.ball.spin;

    mirrored.left = source.right;
    mirrored.right = source.left;
    mirrored.left_score = source.right_score;
    mirrored.right_score = source.left_score;
    return mirrored;
}

whacker::sim::Simulation make_actor_frame_simulation(
    const whacker::sim::Simulation& simulation,
    const bool for_left_paddle) {
    whacker::sim::Simulation actor_frame {simulation.config()};
    actor_frame.mutable_state() = for_left_paddle
        ? simulation.state()
        : mirror_rally_state_x(simulation.config(), simulation.state());
    return actor_frame;
}

AiDecision actor_decision_to_world(const AiDecision& actor_decision, const bool for_left_paddle) {
    if (for_left_paddle) {
        return actor_decision;
    }

    AiDecision world = actor_decision;
    world.strike_feedforward_vy = -world.strike_feedforward_vy;
    world.expected_spin_delta = -world.expected_spin_delta;
    return world;
}

}  // namespace whacker::app::ai_internal
