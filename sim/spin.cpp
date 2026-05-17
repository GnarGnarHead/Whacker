#include "sim/spin.hpp"

#include <cmath>

#include "sim/math.hpp"

namespace whacker::sim {

void apply_spin_curve(BallState& ball, const SimulationConfig& config, const float dt) {
    const float speed = speed_of(ball.velocity);
    if (speed <= 1.0e-6f) {
        return;
    }

    // Magnus-like curvature scales with horizontal travel component so trajectories do not collapse vertically.
    const float horizontal_ratio = clampf(ball.velocity.x / speed, -1.0f, 1.0f);
    const float spin_ratio = (config.spin_max > 1.0e-6f) ? clampf(ball.spin / config.spin_max, -1.0f, 1.0f) : 0.0f;
    const float speed_ref = std::max(config.ball_base_speed, 1.0e-3f);
    const float speed_ratio = speed / speed_ref;
    const float curve_speed_scale =
        std::pow(std::max(speed_ratio, 0.0f), std::max(config.curve_speed_exponent, 0.0f));
    ball.velocity.y += spin_ratio * config.k_curve * horizontal_ratio * curve_speed_scale * dt;
}

void decay_spin(BallState& ball, const SimulationConfig& config, const float dt) {
    if (config.tau_spin <= 0.0f) {
        ball.spin = 0.0f;
        return;
    }
    const float speed = speed_of(ball.velocity);
    const float speed_ref = std::max(config.ball_base_speed, 1.0e-3f);
    const float speed_ratio = speed / speed_ref;
    const float speed_excess = std::max(speed_ratio - 1.0f, 0.0f);
    const float burn_multiplier = 1.0f + (std::max(config.spin_burn_speed_gain, 0.0f) * speed_excess);
    const float decay = std::exp(-(dt / config.tau_spin) * burn_multiplier);
    ball.spin *= decay;
}

void decay_speed_scalar(BallState& ball, const SimulationConfig& config, const float dt) {
    if (config.tau_speed <= 0.0f) {
        ball.speed_scalar = 1.0f;
        return;
    }
    const float decay = std::exp(-dt / config.tau_speed);
    ball.speed_scalar = 1.0f + ((ball.speed_scalar - 1.0f) * decay);
    if (config.ball_speed_scalar_cap > 0.0f) {
        ball.speed_scalar = clampf(ball.speed_scalar, 1.0f, config.ball_speed_scalar_cap);
    } else {
        ball.speed_scalar = std::max(ball.speed_scalar, 1.0f);
    }
}

void inject_spin(
    BallState& ball,
    const SimulationConfig& config,
    const float paddle_velocity,
    const float shot_direction_x,
    const float spin_scale) {
    ball.spin += contact_spin_delta(config, paddle_velocity, spin_scale, shot_direction_x);
    ball.spin = clampf(ball.spin, -config.spin_max, config.spin_max);
}

}  // namespace whacker::sim
