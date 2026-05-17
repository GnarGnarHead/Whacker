#include "sim/physics.hpp"

#include <algorithm>

#include "sim/collision.hpp"
#include "sim/math.hpp"
#include "sim/spin.hpp"

namespace whacker::sim {

Simulation::Simulation(SimulationConfig config)
    : config_(config) {
    reset();
}

void Simulation::reset() {
    state_ = {};
    const float center_y = 0.5f * config_.court_height;
    state_.left.center_y = center_y;
    state_.left.target_y = center_y;
    state_.right.center_y = center_y;
    state_.right.target_y = center_y;
    reset_ball(true);
}

ScoreEvent Simulation::step(const float dt) {
    integrate_paddle(state_.left, dt);
    integrate_paddle(state_.right, dt);

    decay_speed_scalar(state_.ball, config_, dt);
    apply_spin_curve(state_.ball, config_, dt);
    const float target_speed = config_.ball_base_speed * state_.ball.speed_scalar;
    renormalize_velocity(state_.ball, target_speed);
    const Vec2 previous_ball_position = state_.ball.position;
    state_.ball.position.x += state_.ball.velocity.x * dt;
    state_.ball.position.y += state_.ball.velocity.y * dt;
    decay_spin(state_.ball, config_, dt);

    if (handle_wall_bounce(state_.ball, config_)) {
        const float post_wall_target_speed = config_.ball_base_speed * state_.ball.speed_scalar;
        renormalize_velocity(state_.ball, post_wall_target_speed);
    }

    float left_contact_u = 0.0f;
    float right_contact_u = 0.0f;
    const float left_power_scale = power_scale_for(state_.left);
    const float right_power_scale = power_scale_for(state_.right);
    const float left_max_speed = paddle_max_speed_for(config_, state_.left);
    const float right_max_speed = paddle_max_speed_for(config_, state_.right);
    const float left_impact_factor = impact_power_factor(state_.left.velocity_y, left_max_speed);
    const float right_impact_factor = impact_power_factor(state_.right.velocity_y, right_max_speed);
    const float left_spin_at_contact = spin_scale_at_contact(
        config_,
        spin_scale_for(state_.left),
        left_power_scale,
        left_impact_factor);
    const float right_spin_at_contact = spin_scale_at_contact(
        config_,
        spin_scale_for(state_.right),
        right_power_scale,
        right_impact_factor);
    const float left_technical_at_contact = technical_scale_at_contact(
        config_,
        technical_scale_for(state_.left),
        left_power_scale,
        left_impact_factor);
    const float right_technical_at_contact = technical_scale_at_contact(
        config_,
        technical_scale_for(state_.right),
        right_power_scale,
        right_impact_factor);
    const bool left_hit =
        handle_paddle_collision(
            state_.ball,
            previous_ball_position,
            state_.left,
            config_,
            true,
            &left_contact_u,
            left_technical_at_contact,
            left_spin_at_contact);
    const bool right_hit =
        handle_paddle_collision(
            state_.ball,
            previous_ball_position,
            state_.right,
            config_,
            false,
            &right_contact_u,
            right_technical_at_contact,
            right_spin_at_contact);

    if (left_hit || right_hit) {
        const PaddleState& hitter = left_hit ? state_.left : state_.right;
        const float shot_direction_x = left_hit ? 1.0f : -1.0f;
        const float contact_u = left_hit ? left_contact_u : right_contact_u;
        const float hitter_power_scale = left_hit ? left_power_scale : right_power_scale;
        const float hitter_technical_intent = technical_scale_for(hitter);
        const float hitter_spin_intent = spin_scale_for(hitter);
        const float hitter_max_speed = paddle_max_speed_for(config_, hitter);
        const float impact_factor = impact_power_factor(hitter.velocity_y, hitter_max_speed);
        const float spin_scale = spin_scale_at_contact(
            config_,
            hitter_spin_intent,
            hitter_power_scale,
            impact_factor);
        const float spin_commit = contact_spin_commit(hitter_spin_intent, impact_factor);
        const float technical_commit = contact_technical_commit(hitter_technical_intent, contact_u);
        const float energy_diversion = contact_energy_diversion(config_, spin_commit, technical_commit);
        const float powered_speed_scalar = speed_scalar_after_contact(
            state_.ball.speed_scalar,
            config_.ramp_rate,
            contact_u,
            hitter.velocity_y,
            hitter_max_speed,
            config_.power_contact_boost,
            hitter_power_scale,
            config_.ball_speed_scalar_cap,
            energy_diversion);
        const PowerSpinTransfer transfer =
            apply_power_spin_transfer(
                config_,
                powered_speed_scalar,
                hitter_spin_intent,
                spin_commit,
                hitter.velocity_y,
                shot_direction_x);
        const float pre_contact_spin = state_.ball.spin;
        const float intended_spin_delta =
            contact_spin_delta(config_, hitter.velocity_y, spin_scale, shot_direction_x) + transfer.spin_delta;
        inject_spin(state_.ball, config_, hitter.velocity_y, shot_direction_x, spin_scale);
        state_.ball.spin = clampf(state_.ball.spin + transfer.spin_delta, -config_.spin_max, config_.spin_max);
        state_.ball.speed_scalar = speed_scalar_after_counter_spin_recovery(
            config_,
            transfer.speed_scalar,
            pre_contact_spin,
            state_.ball.spin,
            intended_spin_delta,
            contact_u,
            impact_factor,
            hitter_power_scale,
            config_.ball_speed_scalar_cap);
        renormalize_velocity(state_.ball, config_.ball_base_speed * state_.ball.speed_scalar);
        ++state_.rally_hits;
    }

    const ScoreEvent score = handle_scoring(state_.ball, config_);
    if (score == ScoreEvent::LeftPlayerScored) {
        ++state_.left_score;
        state_.rally_hits = 0;
        reset_ball(true);
    } else if (score == ScoreEvent::RightPlayerScored) {
        ++state_.right_score;
        state_.rally_hits = 0;
        reset_ball(false);
    }

    return score;
}

const SimulationConfig& Simulation::config() const {
    return config_;
}

const RallyState& Simulation::state() const {
    return state_;
}

RallyState& Simulation::mutable_state() {
    return state_;
}

void Simulation::integrate_paddle(PaddleState& paddle, const float dt) {
    const float paddle_max_speed = paddle_max_speed_for(config_, paddle);
    const float paddle_accel = paddle_accel_for(config_, paddle);
    constexpr float kPaddleTargetGain = 11.0f;
    const float position_velocity = (paddle.target_y - paddle.center_y) * kPaddleTargetGain;
    const float desired_velocity = clampf(
        position_velocity + paddle.feedforward_velocity_y,
        -paddle_max_speed,
        paddle_max_speed);
    const float max_delta = paddle_accel * dt;
    const float velocity_delta = clampf(desired_velocity - paddle.velocity_y, -max_delta, max_delta);
    paddle.velocity_y += velocity_delta;
    paddle.center_y += paddle.velocity_y * dt;

    const float min_y = config_.paddle_half_height;
    const float max_y = config_.court_height - config_.paddle_half_height;
    if (paddle.center_y < min_y) {
        paddle.center_y = min_y;
        paddle.velocity_y = 0.0f;
    } else if (paddle.center_y > max_y) {
        paddle.center_y = max_y;
        paddle.velocity_y = 0.0f;
    }

    // Feedforward is an explicit per-step command set by the caller.
    paddle.feedforward_velocity_y = 0.0f;
}

void Simulation::reset_ball(const bool serve_to_right) {
    state_.ball.position.x = 0.5f * config_.court_width;
    state_.ball.position.y = 0.5f * config_.court_height;
    state_.ball.spin = 0.0f;
    state_.ball.speed_scalar = 1.0f;
    state_.ball.velocity.x = serve_to_right ? config_.ball_base_speed : -config_.ball_base_speed;
    state_.ball.velocity.y = 0.0f;
}

}  // namespace whacker::sim
