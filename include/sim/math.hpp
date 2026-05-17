#pragma once

#include <algorithm>
#include <cmath>

#include "sim/config.hpp"
#include "sim/types.hpp"

namespace whacker::sim {

inline float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

inline float clamp01(const float value) {
    return clampf(value, 0.0f, 1.0f);
}

inline float speed_of(const Vec2& velocity) {
    return std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
}

inline float speed_of(const BallState& ball) {
    return speed_of(ball.velocity);
}

inline float power_scale_for(const PaddleState& paddle) {
    return clamp01(paddle.power_scale);
}

inline float technical_scale_for(const PaddleState& paddle) {
    return clamp01(paddle.technical_scale);
}

inline float spin_scale_for(const PaddleState& paddle) {
    return clamp01(paddle.spin_scale);
}

inline float paddle_max_speed_for(const SimulationConfig& config, const PaddleState& paddle) {
    (void)paddle;
    return std::max(1.0e-3f, config.paddle_max_speed);
}

inline float paddle_accel_for(const SimulationConfig& config, const PaddleState& paddle) {
    (void)paddle;
    return std::max(1.0e-3f, config.paddle_accel);
}

inline float center_contact_factor(const float contact_u) {
    constexpr float kEdgeDriveFloor = 0.38f;
    const float center_fraction = clampf(1.0f - std::abs(contact_u), 0.0f, 1.0f);
    // Edge contacts still carry some power so rallies do not collapse into flat exchanges.
    return kEdgeDriveFloor + ((1.0f - kEdgeDriveFloor) * center_fraction);
}

inline float impact_power_factor(const float impact_velocity_y, const float paddle_max_speed) {
    const float linear = clampf(
        std::abs(impact_velocity_y) / std::max(paddle_max_speed, 1.0e-3f),
        0.0f,
        1.0f);
    // Amplify mid-range paddle speeds so power shots are meaningful without requiring perfect max-speed contact.
    return std::sqrt(linear);
}

inline float technical_scale_at_contact(
    const SimulationConfig& config,
    const float technical_scale,
    const float power_scale,
    const float impact_factor) {
    (void)config;
    (void)power_scale;
    (void)impact_factor;
    const float skill = clamp01(technical_scale);
    constexpr float kMinTechnicalScale = 0.18f;
    const float skill_curve = std::pow(skill, 1.30f);
    // Edge contact always imparts some angle; technique skill scales how much angle authority the player has.
    return clampf(kMinTechnicalScale + ((1.0f - kMinTechnicalScale) * skill_curve), 0.0f, 1.0f);
}

inline float spin_scale_at_contact(
    const SimulationConfig& config,
    const float spin_scale,
    const float power_scale,
    const float impact_factor) {
    const float base_spin = clamp01(spin_scale);
    const float power_assist =
        clamp01(config.power_to_spin_coupling) * clamp01(power_scale) * clamp01(impact_factor);
    return clampf(
        base_spin * (1.0f + (0.25f * power_assist)),
        0.0f,
        2.5f);
}

inline float contact_spin_commit(const float spin_scale, const float impact_factor) {
    return clamp01(spin_scale * impact_factor);
}

inline float contact_technical_commit(const float technical_scale, const float contact_u) {
    return clamp01(technical_scale * std::abs(contact_u));
}

inline float contact_energy_diversion(
    const SimulationConfig& config,
    const float spin_commit,
    const float technical_commit) {
    return clampf(
        (config.power_energy_spin_drag * clamp01(spin_commit)) +
            (config.power_energy_technical_drag * clamp01(technical_commit)),
        0.0f,
        0.92f);
}

struct PowerSpinTransfer {
    float speed_scalar = 1.0f;
    float spin_delta = 0.0f;
};

inline float contact_spin_delta(
    const SimulationConfig& config,
    const float paddle_velocity_y,
    const float spin_scale,
    const float shot_direction_x) {
    const float max_speed = std::max(config.paddle_max_speed, 1.0e-3f);
    const float speed_ratio = std::pow(clampf(std::abs(paddle_velocity_y) / max_speed, 0.0f, 1.0f), 0.80f);
    const float spin_skill = std::pow(clamp01(spin_scale), 1.65f);
    const float paddle_sign = paddle_velocity_y >= 0.0f ? 1.0f : -1.0f;
    const float shot_sign = shot_direction_x >= 0.0f ? 1.0f : -1.0f;
    // "Up stroke adds topspin" from both sides:
    // with +Y downward, this means spin sign depends on both paddle motion and shot direction.
    const float spin_direction = -paddle_sign * shot_sign;
    return spin_direction * config.k_take * max_speed * speed_ratio * spin_skill;
}

inline PowerSpinTransfer apply_power_spin_transfer(
    const SimulationConfig& config,
    const float speed_scalar_after_contact,
    const float spin_intent,
    const float spin_commit,
    const float paddle_velocity_y,
    const float shot_direction_x) {
    PowerSpinTransfer out {};
    out.speed_scalar = std::max(1.0f, speed_scalar_after_contact);

    const float transfer_drive =
        clampf(clamp01(config.power_spin_transfer_ratio) * clamp01(spin_intent) * clamp01(spin_commit), 0.0f, 0.95f);
    if (transfer_drive <= 1.0e-6f || std::abs(paddle_velocity_y) <= 1.0e-6f) {
        return out;
    }

    const float excess_speed = std::max(0.0f, out.speed_scalar - 1.0f);
    if (excess_speed <= 1.0e-6f) {
        return out;
    }

    const float transferred_speed = excess_speed * transfer_drive;
    out.speed_scalar = std::max(1.0f, out.speed_scalar - transferred_speed);

    const float speed_span = std::max(config.ball_speed_scalar_cap - 1.0f, 1.0e-3f);
    const float normalized_transfer = clampf(transferred_speed / speed_span, 0.0f, 1.0f);
    const float paddle_sign = paddle_velocity_y >= 0.0f ? 1.0f : -1.0f;
    const float shot_sign = shot_direction_x >= 0.0f ? 1.0f : -1.0f;
    const float spin_direction = -paddle_sign * shot_sign;
    out.spin_delta = spin_direction * normalized_transfer * config.spin_max * clamp01(config.power_to_spin_coupling);
    return out;
}

inline float speed_scalar_after_counter_spin_recovery(
    const SimulationConfig& config,
    const float current_speed_scalar,
    const float pre_contact_spin,
    const float post_contact_spin,
    const float spin_delta_applied,
    const float contact_u,
    const float impact_factor,
    const float power_scale,
    const float speed_scalar_cap = 0.0f) {
    const float clamped_current = std::max(1.0f, current_speed_scalar);
    if ((config.spin_counter_power_ratio <= 0.0f) || (config.spin_max <= 1.0e-6f)) {
        if (speed_scalar_cap > 1.0f) {
            return clampf(clamped_current, 1.0f, speed_scalar_cap);
        }
        return clamped_current;
    }

    // Recovery only occurs when the paddle applies spin opposite to the incoming spin.
    if ((pre_contact_spin * spin_delta_applied) >= -1.0e-6f) {
        if (speed_scalar_cap > 1.0f) {
            return clampf(clamped_current, 1.0f, speed_scalar_cap);
        }
        return clamped_current;
    }

    const float canceled_spin = std::max(0.0f, std::abs(pre_contact_spin) - std::abs(post_contact_spin));
    if (canceled_spin <= 1.0e-6f) {
        if (speed_scalar_cap > 1.0f) {
            return clampf(clamped_current, 1.0f, speed_scalar_cap);
        }
        return clamped_current;
    }

    const float cancel_ratio = clampf(canceled_spin / std::max(config.spin_max, 1.0e-3f), 0.0f, 1.0f);
    const float center_factor = center_contact_factor(contact_u);
    const float weighted_center = 0.35f + (0.65f * center_factor);
    const float weighted_impact = 0.35f + (0.65f * clamp01(impact_factor));
    const float weighted_power = 0.25f + (0.75f * clamp01(power_scale));
    float gain = clamp01(config.spin_counter_power_ratio) * cancel_ratio * weighted_center * weighted_impact *
        weighted_power;
    gain = std::min(gain, clamp01(config.spin_counter_power_cap));

    if (speed_scalar_cap > 1.0f) {
        const float capped_current = clampf(clamped_current, 1.0f, speed_scalar_cap);
        const float headroom = std::max(0.0f, speed_scalar_cap - capped_current);
        constexpr float kCounterApproachGain = 0.95f;
        const float approach = 1.0f - std::exp(-(gain * kCounterApproachGain));
        return capped_current + (headroom * approach);
    }

    return clamped_current * (1.0f + gain);
}

inline float speed_scalar_after_contact(
    const float current_speed_scalar,
    const float ramp_rate,
    const float contact_u,
    const float impact_velocity_y = 0.0f,
    const float paddle_max_speed = 1.0f,
    const float power_contact_boost = 0.0f,
    const float power_scale = 1.0f,
    const float speed_scalar_cap = 0.0f,
    const float energy_diversion = 0.0f) {
    const float center_factor = center_contact_factor(contact_u);
    const float power_factor = impact_power_factor(impact_velocity_y, paddle_max_speed);
    const float power_skill = clamp01(power_scale);
    const float power_curve = std::pow(power_skill, 1.65f);
    // Center contact always contributes some drive, and both base drive and velocity amplification
    // scale with power skill.
    const float center_drive = 0.03f + (0.45f * power_curve);
    const float velocity_drive = 0.55f * power_curve * power_factor;
    const float contact_drive = center_factor * (center_drive + velocity_drive);
    const float gain = ramp_rate * (power_contact_boost * contact_drive) *
        (1.0f - clampf(energy_diversion, 0.0f, 0.95f));

    const float non_negative_gain = std::max(0.0f, gain);
    if (speed_scalar_cap > 1.0f) {
        const float capped_current = clampf(current_speed_scalar, 1.0f, speed_scalar_cap);
        const float headroom = std::max(0.0f, speed_scalar_cap - capped_current);
        constexpr float kTerminalApproachGain = 0.50f;
        // Convert contact gain into a smooth "approach fraction" so repeated power shots
        // accelerate hard early and naturally saturate near terminal velocity.
        const float approach = 1.0f - std::exp(-(non_negative_gain * kTerminalApproachGain));
        return capped_current + (headroom * approach);
    }

    return current_speed_scalar * (1.0f + non_negative_gain);
}

inline void renormalize_velocity(
    BallState& ball,
    const float target_speed,
    const float fallback_direction_x = 1.0f) {
    const float current_speed = speed_of(ball);
    if (current_speed <= 1.0e-6f) {
        ball.velocity.x = fallback_direction_x >= 0.0f ? target_speed : -target_speed;
        ball.velocity.y = 0.0f;
        return;
    }

    const float scale = target_speed / current_speed;
    ball.velocity.x *= scale;
    ball.velocity.y *= scale;
}

}  // namespace whacker::sim
