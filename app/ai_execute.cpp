#include "ai_execute.hpp"

namespace whacker::app::ai_internal {

void apply_ai_decision_to_paddle(
    whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config,
    const AiDecision& decision,
    const bool inbound,
    const float intercept_time_s,
    const bool ambient_mode) {
    const float min_y = config.paddle_half_height;
    const float max_y = config.court_height - config.paddle_half_height;

    if (!decision.valid) {
        paddle.target_y = clampf(0.5f * config.court_height, min_y, max_y);
        paddle.feedforward_velocity_y = 0.0f;
        return;
    }

    if (inbound) {
        paddle.target_y = clampf(decision.pre_contact_target_y, min_y, max_y);
        if (ambient_mode || decision.intent == AiIntent::Stabilize) {
            paddle.feedforward_velocity_y = 0.0f;
            return;
        }

        const float technical_hint = clamp01f(decision.style_mix_technical);
        const float alignment_threshold = clampf(
            (0.24f - (0.10f * technical_hint)) * config.paddle_half_height,
            2.0f,
            7.0f);
        const float alignment_error = std::abs(paddle.center_y - paddle.target_y);
        if (alignment_error > alignment_threshold) {
            paddle.feedforward_velocity_y = 0.0f;
            return;
        }

        const float strike_window_s = std::max(0.0f, decision.strike_commit_window_s);
        const float min_make = clamp01f(decision.strike_min_make_prob);
        const bool high_risk_spin_commit =
            decision.intent == AiIntent::SpinTrap &&
            decision.style_mix_spin >= 0.55f &&
            decision.make_contact_probability >= (min_make + 0.08f);
        if (strike_window_s <= 1.0e-6f ||
            intercept_time_s >= strike_window_s ||
            decision.make_contact_probability < min_make ||
            (decision.miss_risk_level >= 2 && !high_risk_spin_commit)) {
            paddle.feedforward_velocity_y = 0.0f;
            return;
        }

        const float target_abs = std::max(
            0.0f,
            decision.strike_velocity_target_abs > 0.0f
                ? decision.strike_velocity_target_abs
                : std::abs(decision.strike_feedforward_vy));
        if (target_abs <= 1.0e-6f) {
            paddle.feedforward_velocity_y = 0.0f;
            return;
        }

        const float phase = clamp01f(1.0f - (intercept_time_s / std::max(strike_window_s, 1.0e-3f)));
        const float gain = decision.intent == AiIntent::SpinTrap
            ? (0.55f + (0.45f * phase))
            : (0.65f + (0.35f * phase));

        const float strike_sign = signf(decision.strike_feedforward_vy);
        const float signed_strike = (strike_sign == 0.0f ? 1.0f : strike_sign) * (target_abs * gain);
        paddle.feedforward_velocity_y = clampf(
            signed_strike,
            -config.paddle_max_speed,
            config.paddle_max_speed);
        return;
    }

    paddle.target_y = clampf(decision.post_contact_recover_y, min_y, max_y);
    const float gain = ambient_mode ? 3.0f : 5.0f;
    const float cap = config.paddle_max_speed * (ambient_mode ? 0.30f : 0.40f);
    const float feedforward = (paddle.target_y - paddle.center_y) * gain;
    paddle.feedforward_velocity_y = clampf(feedforward, -cap, cap);
}

}  // namespace whacker::app::ai_internal
