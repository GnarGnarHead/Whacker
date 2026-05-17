#include "ai_plan_state_bridge.hpp"

#include <algorithm>

namespace {

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

int clampi(const int value, const int lo, const int hi) {
    return std::max(lo, std::min(value, hi));
}

std::uint8_t intent_to_id(const whacker::app::AiIntent intent) {
    return static_cast<std::uint8_t>(intent);
}

whacker::app::AiIntent intent_from_id(const std::uint8_t intent_id) {
    switch (intent_id) {
        case 1U:
            return whacker::app::AiIntent::Pressure;
        case 2U:
            return whacker::app::AiIntent::SpinTrap;
        case 0U:
        default:
            return whacker::app::AiIntent::Stabilize;
    }
}

}  // namespace

namespace whacker::app {

AiDecision runtime_ai_decision_from_plan(const RuntimeAiPlanState& plan) {
    AiDecision decision {};
    decision.valid = plan.has_plan;
    decision.inbound = plan.ball_was_inbound;
    decision.intent = intent_from_id(plan.intent_id);
    decision.candidate_id = plan.candidate_id;
    decision.intercept_time_s = plan.intercept_time_s;
    decision.intercept_y = plan.intercept_y;
    decision.contact_u = plan.contact_u;
    decision.strike_feedforward_vy = plan.strike_feedforward_vy;
    decision.pre_contact_target_y = plan.pre_contact_target_y;
    decision.post_contact_recover_y = plan.post_contact_recover_y;
    decision.confidence = plan.confidence;
    decision.score = plan.decision_score;
    decision.coarse_candidate_count = plan.coarse_candidate_count;
    decision.scored_candidate_count = plan.scored_candidate_count;
    decision.reachable_candidate_count = plan.reachable_candidate_count;
    decision.predicted_wall_bounces = plan.predicted_wall_bounces;
    decision.make_contact_probability = plan.make_contact_probability;
    decision.reach_slack = plan.reach_slack;
    decision.miss_risk_level = plan.miss_risk_level;
    decision.expected_impact_factor = plan.expected_impact_factor;
    decision.expected_spin_delta = plan.expected_spin_delta;
    decision.clean_contact_metric = plan.clean_contact_metric;
    decision.style_mix_power = plan.style_mix_power;
    decision.style_mix_technical = plan.style_mix_technical;
    decision.style_mix_spin = plan.style_mix_spin;
    decision.intent_weight_stabilize = plan.intent_weight_stabilize;
    decision.intent_weight_pressure = plan.intent_weight_pressure;
    decision.intent_weight_spintrap = plan.intent_weight_spintrap;
    decision.strike_commit_window_s = plan.strike_commit_window_s;
    decision.strike_min_make_prob = plan.strike_min_make_prob;
    decision.strike_velocity_target_abs = plan.strike_velocity_target_abs;
    return decision;
}

void write_runtime_ai_plan_from_decision(
    RuntimeAiState& ai_state,
    const AiDecision& decision,
    const std::uint64_t state_signature,
    const bool inbound) {
    RuntimeAiPlanState plan {};
    if (!decision.valid) {
        ai_state.plan = plan;
        return;
    }

    plan.has_plan = true;
    plan.plan_created_step = ai_state.runtime_step_counter;
    plan.valid_until_step = ai_state.runtime_step_counter + clampi(decision.valid_steps, 6, 84);
    plan.replan_cooldown_steps = clampi(decision.cooldown_steps, 2, 70);
    plan.intercept_time_s = std::max(decision.intercept_time_s, 0.0f);
    plan.intercept_y = decision.intercept_y;
    plan.contact_u = decision.contact_u;
    plan.strike_feedforward_vy = decision.strike_feedforward_vy;
    plan.pre_contact_target_y = decision.pre_contact_target_y;
    plan.post_contact_recover_y = decision.post_contact_recover_y;
    plan.confidence = clampf(decision.confidence, 0.0f, 1.0f);
    plan.decision_score = decision.score;
    plan.state_signature = state_signature;
    plan.ball_was_inbound = inbound;
    plan.intent_id = intent_to_id(decision.intent);
    plan.candidate_id = decision.candidate_id;
    plan.coarse_candidate_count = decision.coarse_candidate_count;
    plan.scored_candidate_count = decision.scored_candidate_count;
    plan.reachable_candidate_count = decision.reachable_candidate_count;
    plan.predicted_wall_bounces = decision.predicted_wall_bounces;
    plan.make_contact_probability = decision.make_contact_probability;
    plan.reach_slack = decision.reach_slack;
    plan.miss_risk_level = decision.miss_risk_level;
    plan.expected_impact_factor = decision.expected_impact_factor;
    plan.expected_spin_delta = decision.expected_spin_delta;
    plan.clean_contact_metric = decision.clean_contact_metric;
    plan.style_mix_power = decision.style_mix_power;
    plan.style_mix_technical = decision.style_mix_technical;
    plan.style_mix_spin = decision.style_mix_spin;
    plan.intent_weight_stabilize = decision.intent_weight_stabilize;
    plan.intent_weight_pressure = decision.intent_weight_pressure;
    plan.intent_weight_spintrap = decision.intent_weight_spintrap;
    plan.strike_commit_window_s = decision.strike_commit_window_s;
    plan.strike_min_make_prob = decision.strike_min_make_prob;
    plan.strike_velocity_target_abs = decision.strike_velocity_target_abs;
    ai_state.plan = plan;
}

}  // namespace whacker::app

