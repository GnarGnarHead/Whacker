#include "sim/collision.hpp"

#include <algorithm>
#include <cmath>

#include "sim/math.hpp"

namespace whacker::sim {

namespace {

void apply_wall_spin_response(
    BallState& ball,
    const SimulationConfig& config,
    const float wall_normal_sign,
    const float pre_bounce_vy_magnitude) {
    // Tangential slip at contact combines translational x-speed and spin-induced surface speed.
    const float tangential_scale = std::max(config.k_wall_spin_tangent, 0.0f);
    if (tangential_scale <= 1.0e-6f) {
        ball.spin = clampf(ball.spin * config.wall_spin_retention, -config.spin_max, config.spin_max);
        return;
    }

    const float speed_ref = std::max(config.ball_base_speed, 1.0e-3f);
    const float impact_ratio = clampf(pre_bounce_vy_magnitude / speed_ref, 0.0f, 2.0f);
    const float tangent_weight = clampf(tangential_scale / 16.0f, 0.0f, 2.0f);
    const float spin_strength = clampf(std::abs(ball.spin) / std::max(config.spin_max, 1.0e-3f), 0.0f, 1.0f);
    const float clamped_impact = clampf(impact_ratio, 0.0f, 1.0f);
    const float spin_quadratic = spin_strength * spin_strength;
    const float friction_coupling = clampf(
        (0.22f + (0.30f * clamped_impact)) * tangent_weight * spin_quadratic,
        0.0f,
        0.50f);

    const float pre_linear_speed = speed_of(ball.velocity);
    const float tangential_slip = ball.velocity.x - (wall_normal_sign * ball.spin * tangential_scale);
    const float tangential_impulse = -friction_coupling * tangential_slip;
    ball.velocity.x += tangential_impulse;

    // Frictional impulse couples linear slip into opposite spin torque at the wall contact point.
    // Cap per-bounce spin exchange so a single wall hit cannot dump most rotational energy.
    const float spin_exchange_gain = 0.42f / tangential_scale;
    const float raw_spin_delta = -wall_normal_sign * tangential_impulse * spin_exchange_gain;
    const float max_spin_exchange = std::abs(ball.spin) * 0.14f;
    ball.spin += clampf(raw_spin_delta, -max_spin_exchange, max_spin_exchange);
    ball.spin = clampf(ball.spin * config.wall_spin_retention, -config.spin_max, config.spin_max);

    const float post_linear_speed = speed_of(ball.velocity);
    if (pre_linear_speed > 1.0e-6f && post_linear_speed > 1.0e-6f) {
        const float linear_ratio = clampf(post_linear_speed / pre_linear_speed, 0.94f, 1.06f);
        ball.speed_scalar = std::max(1.0f, ball.speed_scalar * linear_ratio);
        if (config.ball_speed_scalar_cap > 1.0f) {
            ball.speed_scalar = clampf(ball.speed_scalar, 1.0f, config.ball_speed_scalar_cap);
        }
    }
}

}  // namespace

bool handle_wall_bounce(BallState& ball, const SimulationConfig& config) {
    const float min_y = config.ball_radius;
    const float max_y = config.court_height - config.ball_radius;
    bool bounced = false;

    if (ball.position.y < min_y && ball.velocity.y < 0.0f) {
        const float pre_bounce_vy = ball.velocity.y;
        ball.position.y = min_y;
        ball.velocity.y = -ball.velocity.y;
        apply_wall_spin_response(ball, config, 1.0f, std::abs(pre_bounce_vy));
        bounced = true;
    }
    if (ball.position.y > max_y && ball.velocity.y > 0.0f) {
        const float pre_bounce_vy = ball.velocity.y;
        ball.position.y = max_y;
        ball.velocity.y = -ball.velocity.y;
        apply_wall_spin_response(ball, config, -1.0f, std::abs(pre_bounce_vy));
        bounced = true;
    }

    return bounced;
}

ScoreEvent handle_scoring(const BallState& ball, const SimulationConfig& config) {
    if (ball.position.x < -config.ball_radius) {
        return ScoreEvent::RightPlayerScored;
    }
    if (ball.position.x > config.court_width + config.ball_radius) {
        return ScoreEvent::LeftPlayerScored;
    }
    return ScoreEvent::None;
}

bool handle_paddle_collision(
    BallState& ball,
    const Vec2& previous_position,
    const PaddleState& paddle,
    const SimulationConfig& config,
    const bool is_left_paddle,
    float* contact_u_out,
    const float technical_scale,
    const float spin_scale) {
    const float paddle_x =
        is_left_paddle ? config.paddle_x_margin : (config.court_width - config.paddle_x_margin);
    const float paddle_left = paddle_x - config.paddle_half_width;
    const float paddle_right = paddle_x + config.paddle_half_width;
    const float ball_left = ball.position.x - config.ball_radius;
    const float ball_right = ball.position.x + config.ball_radius;
    const float contact_plane =
        is_left_paddle ? (paddle_right + config.ball_radius) : (paddle_left - config.ball_radius);

    const bool moving_toward_paddle = is_left_paddle ? (ball.velocity.x < 0.0f) : (ball.velocity.x > 0.0f);
    if (!moving_toward_paddle) {
        return false;
    }

    bool crossed_contact_plane = false;
    float sweep_t = 1.0f;
    const float delta_x = ball.position.x - previous_position.x;

    if (std::abs(delta_x) > 1.0e-6f) {
        if (is_left_paddle) {
            crossed_contact_plane = (previous_position.x > contact_plane) && (ball.position.x <= contact_plane);
        } else {
            crossed_contact_plane = (previous_position.x < contact_plane) && (ball.position.x >= contact_plane);
        }

        if (crossed_contact_plane) {
            sweep_t = clampf((contact_plane - previous_position.x) / delta_x, 0.0f, 1.0f);
        }
    }

    const bool overlap_x = (ball_left <= paddle_right) && (ball_right >= paddle_left);
    if (!crossed_contact_plane) {
        if (!overlap_x) {
            return false;
        }

        // Overlap fallback is only valid while the ball is still on the approach side.
        // Without this guard, a ball already behind the paddle can be "rescued" into a hit.
        constexpr float kPlaneEpsilon = 1.0e-4f;
        const bool approach_side_ok = is_left_paddle
            ? ((previous_position.x >= (contact_plane - kPlaneEpsilon)) &&
                (ball.position.x >= (contact_plane - kPlaneEpsilon)))
            : ((previous_position.x <= (contact_plane + kPlaneEpsilon)) &&
                (ball.position.x <= (contact_plane + kPlaneEpsilon)));
        if (!approach_side_ok) {
            return false;
        }
    }

    const float y_at_hit = previous_position.y + ((ball.position.y - previous_position.y) * sweep_t);
    const float dy = std::abs(y_at_hit - paddle.center_y);
    if (dy > config.paddle_half_height + config.ball_radius) {
        return false;
    }

    const float u = clampf((y_at_hit - paddle.center_y) / config.paddle_half_height, -1.0f, 1.0f);
    if (contact_u_out != nullptr) {
        *contact_u_out = u;
    }
    const float surface_spin_sign = is_left_paddle ? 1.0f : -1.0f;
    const float spin_contact_ratio = ball.spin / std::max(config.spin_max, 1.0e-3f);
    const float effective_u =
        clampf(u + (surface_spin_sign * spin_contact_ratio * config.k_spin_contact_bias), -1.0f, 1.0f);
    const float theta = effective_u * config.theta_max_rad * clamp01(technical_scale);
    const float hit_speed = config.ball_base_speed * ball.speed_scalar;
    const float direction = is_left_paddle ? 1.0f : -1.0f;

    ball.velocity.x = direction * hit_speed * std::cos(theta);
    ball.velocity.y = hit_speed * std::sin(theta);
    const float spin_ratio = clampf(ball.spin / std::max(config.spin_max, 1.0e-3f), -1.0f, 1.0f);
    const float rebound_scale = 0.35f + (0.45f * clamp01(spin_scale));
    const float raw_spin_kick = surface_spin_sign * spin_ratio * config.spin_max * config.k_spin_rebound * rebound_scale;
    const float max_spin_kick = hit_speed * (0.14f + (0.16f * clamp01(spin_scale)));
    ball.velocity.y += clampf(raw_spin_kick, -max_spin_kick, max_spin_kick);
    ball.spin *= config.paddle_spin_retention;
    ball.position.y = clampf(y_at_hit, config.ball_radius, config.court_height - config.ball_radius);

    if (is_left_paddle) {
        ball.position.x = contact_plane + 0.1f;
    } else {
        ball.position.x = contact_plane - 0.1f;
    }

    renormalize_velocity(ball, hit_speed, direction);
    return true;
}

}  // namespace whacker::sim
