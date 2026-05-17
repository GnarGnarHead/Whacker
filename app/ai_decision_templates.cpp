#include "ai_decision_templates.hpp"

#include "ai_profile.hpp"

namespace whacker::app::ai_internal {

AiDecision make_recover_decision(
    const whacker::sim::Simulation& simulation,
    const RuntimeAiState& ai_state,
    const float competence,
    const StyleMix& mix,
    const IntentWeights& weights) {
    const auto& config = simulation.config();
    const auto& state = simulation.state();

    AiDecision decision {};
    decision.valid = true;
    decision.inbound = false;
    decision.intent = AiIntent::Stabilize;
    decision.candidate_id = -1;
    decision.intercept_time_s = 0.0f;
    decision.intercept_y = clampf(
        style_recover_lane_y_actor(config, ai_state.style, state.ball.spin),
        config.paddle_half_height,
        config.court_height - config.paddle_half_height);
    decision.contact_u = 0.0f;
    decision.strike_feedforward_vy = 0.0f;
    decision.pre_contact_target_y = 0.5f * config.court_height;
    decision.post_contact_recover_y = decision.intercept_y;
    decision.confidence = 1.0f;
    decision.score = 0.0f;
    decision.valid_steps = 8;
    decision.cooldown_steps = clampi(static_cast<int>(std::lround(2.0f + 4.0f * (1.0f - competence))), 2, 8);

    decision.style_mix_power = mix.power;
    decision.style_mix_technical = mix.technical;
    decision.style_mix_spin = mix.spin;
    decision.intent_weight_stabilize = weights.stabilize;
    decision.intent_weight_pressure = weights.pressure;
    decision.intent_weight_spintrap = weights.spintrap;
    decision.strike_commit_window_s = 0.0f;
    decision.strike_min_make_prob = 1.0f;
    decision.strike_velocity_target_abs = 0.0f;
    return decision;
}

AiDecision make_safe_intercept_decision(
    const whacker::sim::Simulation& simulation,
    const RuntimeAiState& ai_state,
    const float intercept_time_s,
    const float confidence,
    const float competence,
    const float intercept_y,
    const StyleMix& mix,
    const IntentWeights& weights,
    const AiCapabilityProfile& capability) {
    const auto& config = simulation.config();
    const auto& state = simulation.state();
    const auto& self = state.left;

    const float quant_step = capability.target_quantization_step;
    const float emergency_gain = lerpf(0.35f, 0.95f, competence);
    const float blended_target = lerpf(self.center_y, intercept_y, emergency_gain);

    AiDecision decision {};
    decision.valid = true;
    decision.inbound = true;
    decision.intent = AiIntent::Stabilize;
    decision.candidate_id = -1;
    decision.intercept_time_s = std::max(0.0f, intercept_time_s);
    decision.intercept_y = intercept_y;
    decision.contact_u = 0.0f;
    decision.strike_feedforward_vy = 0.0f;
    decision.pre_contact_target_y = clampf(
        std::round(blended_target / quant_step) * quant_step,
        config.paddle_half_height,
        config.court_height - config.paddle_half_height);
    decision.post_contact_recover_y = clampf(
        style_recover_lane_y_actor(config, ai_state.style, state.ball.spin),
        config.paddle_half_height,
        config.court_height - config.paddle_half_height);
    decision.confidence = clamp01f(confidence);
    decision.score = -0.2f;
    decision.valid_steps = clampi(static_cast<int>(std::lround(16.0f + (32.0f * (1.0f - competence)))), 12, 72);
    decision.cooldown_steps = clampi(static_cast<int>(std::lround(3.0f + (6.0f * (1.0f - competence)))), 2, 12);

    decision.make_contact_probability = clamp01f(0.28f + (0.35f * confidence));
    decision.reach_slack = -0.25f;
    decision.miss_risk_level = 2;

    decision.style_mix_power = mix.power;
    decision.style_mix_technical = mix.technical;
    decision.style_mix_spin = mix.spin;
    decision.intent_weight_stabilize = weights.stabilize;
    decision.intent_weight_pressure = weights.pressure;
    decision.intent_weight_spintrap = weights.spintrap;
    decision.strike_commit_window_s = 0.0f;
    decision.strike_min_make_prob = 1.0f;
    decision.strike_velocity_target_abs = 0.0f;
    return decision;
}

}  // namespace whacker::app::ai_internal
