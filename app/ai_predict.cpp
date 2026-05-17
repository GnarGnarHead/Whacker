#include "ai_predict.hpp"

#include "sim/collision.hpp"
#include "sim/math.hpp"
#include "sim/spin.hpp"

namespace whacker::app::ai_internal {

bool moving_toward_actor_paddle(const whacker::sim::RallyState& state) {
    return state.ball.velocity.x < -kInboundEpsilon;
}

float actor_paddle_contact_plane_x(const whacker::sim::SimulationConfig& config) {
    const float paddle_x = config.paddle_x_margin;
    return paddle_x + config.paddle_half_width + config.ball_radius;
}

PredictorResult predict_intercept(
    const whacker::sim::RallyState& state,
    const whacker::sim::SimulationConfig& config,
    const int max_steps) {
    PredictorResult out {};
    const float plane_x = actor_paddle_contact_plane_x(config);

    if (!moving_toward_actor_paddle(state) || (std::abs(state.ball.velocity.x) <= kInboundEpsilon)) {
        return out;
    }

    whacker::sim::BallState probe = state.ball;
    int wall_bounces = 0;

    for (int i = 0; i < max_steps; ++i) {
        const whacker::sim::Vec2 prev = probe.position;
        whacker::sim::decay_speed_scalar(probe, config, whacker::sim::kFixedDt);
        whacker::sim::apply_spin_curve(probe, config, whacker::sim::kFixedDt);
        const float target_speed = config.ball_base_speed * probe.speed_scalar;
        whacker::sim::renormalize_velocity(probe, target_speed, probe.velocity.x >= 0.0f ? 1.0f : -1.0f);
        probe.position.x += probe.velocity.x * whacker::sim::kFixedDt;
        probe.position.y += probe.velocity.y * whacker::sim::kFixedDt;
        whacker::sim::decay_spin(probe, config, whacker::sim::kFixedDt);

        if (whacker::sim::handle_wall_bounce(probe, config)) {
            ++wall_bounces;
            const float post_wall_speed = config.ball_base_speed * probe.speed_scalar;
            whacker::sim::renormalize_velocity(probe, post_wall_speed, probe.velocity.x >= 0.0f ? 1.0f : -1.0f);
        }

        if (whacker::sim::handle_scoring(probe, config) != whacker::sim::ScoreEvent::None) {
            return out;
        }

        const bool crossed = (prev.x > plane_x) && (probe.position.x <= plane_x);
        if (!crossed) {
            continue;
        }

        const float dx = probe.position.x - prev.x;
        const float t = std::abs(dx) > 1.0e-6f ? clampf((plane_x - prev.x) / dx, 0.0f, 1.0f) : 1.0f;
        const float intercept_y = clampf(
            prev.y + (probe.position.y - prev.y) * t,
            config.ball_radius,
            config.court_height - config.ball_radius);
        const float t_hit = (static_cast<float>(i) + t) * whacker::sim::kFixedDt;

        const float c_time = std::exp(-t_hit / 0.70f);
        const float c_bounce = std::exp(-0.30f * static_cast<float>(wall_bounces));

        out.predicted = true;
        out.t_hit = t_hit;
        out.intercept_y = intercept_y;
        out.intercept_vy = probe.velocity.y;
        out.wall_bounces = wall_bounces;
        out.confidence = clamp01f(c_time * c_bounce);
        return out;
    }

    return out;
}

}  // namespace whacker::app::ai_internal
