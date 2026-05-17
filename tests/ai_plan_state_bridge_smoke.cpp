#include <cmath>
#include <cstdlib>

#include "ai_plan_state_bridge.hpp"

namespace {

bool approx_equal(const float a, const float b, const float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

void test_runtime_plan_round_trip_preserves_mapped_fields() {
    whacker::app::RuntimeAiState ai_state {};
    ai_state.runtime_step_counter = 41;

    whacker::app::AiDecision decision {};
    decision.valid = true;
    decision.inbound = true;
    decision.intent = whacker::app::AiIntent::SpinTrap;
    decision.candidate_id = 17;
    decision.intercept_time_s = -0.25f;
    decision.intercept_y = 188.0f;
    decision.contact_u = 0.42f;
    decision.strike_feedforward_vy = -95.0f;
    decision.pre_contact_target_y = 170.0f;
    decision.post_contact_recover_y = 130.0f;
    decision.confidence = 1.20f;
    decision.score = 0.72f;
    decision.valid_steps = 4;
    decision.cooldown_steps = 90;
    decision.coarse_candidate_count = 23;
    decision.scored_candidate_count = 11;
    decision.reachable_candidate_count = 9;
    decision.predicted_wall_bounces = 2;
    decision.make_contact_probability = 0.66f;
    decision.reach_slack = 0.12f;
    decision.miss_risk_level = 1;
    decision.expected_impact_factor = 0.81f;
    decision.expected_spin_delta = -0.37f;
    decision.clean_contact_metric = 0.74f;
    decision.style_mix_power = 0.10f;
    decision.style_mix_technical = 0.25f;
    decision.style_mix_spin = 0.65f;
    decision.intent_weight_stabilize = 0.05f;
    decision.intent_weight_pressure = 0.20f;
    decision.intent_weight_spintrap = 0.75f;
    decision.strike_commit_window_s = 0.18f;
    decision.strike_min_make_prob = 0.38f;
    decision.strike_velocity_target_abs = 115.0f;

    constexpr std::uint64_t kSignature = 0xA1B2C3D4E5F60789ULL;
    whacker::app::write_runtime_ai_plan_from_decision(ai_state, decision, kSignature, true);

    const auto& plan = ai_state.plan;
    require(plan.has_plan);
    require(plan.plan_created_step == 41);
    require(plan.valid_until_step == 47);
    require(plan.replan_cooldown_steps == 70);
    require(approx_equal(plan.intercept_time_s, 0.0f));
    require(approx_equal(plan.intercept_y, decision.intercept_y));
    require(approx_equal(plan.contact_u, decision.contact_u));
    require(approx_equal(plan.strike_feedforward_vy, decision.strike_feedforward_vy));
    require(approx_equal(plan.pre_contact_target_y, decision.pre_contact_target_y));
    require(approx_equal(plan.post_contact_recover_y, decision.post_contact_recover_y));
    require(approx_equal(plan.confidence, 1.0f));
    require(approx_equal(plan.decision_score, decision.score));
    require(plan.state_signature == kSignature);
    require(plan.ball_was_inbound);
    require(plan.intent_id == 2U);
    require(plan.candidate_id == decision.candidate_id);
    require(plan.coarse_candidate_count == decision.coarse_candidate_count);
    require(plan.scored_candidate_count == decision.scored_candidate_count);
    require(plan.reachable_candidate_count == decision.reachable_candidate_count);
    require(plan.predicted_wall_bounces == decision.predicted_wall_bounces);
    require(approx_equal(plan.make_contact_probability, decision.make_contact_probability));
    require(approx_equal(plan.reach_slack, decision.reach_slack));
    require(plan.miss_risk_level == decision.miss_risk_level);
    require(approx_equal(plan.expected_impact_factor, decision.expected_impact_factor));
    require(approx_equal(plan.expected_spin_delta, decision.expected_spin_delta));
    require(approx_equal(plan.clean_contact_metric, decision.clean_contact_metric));
    require(approx_equal(plan.style_mix_power, decision.style_mix_power));
    require(approx_equal(plan.style_mix_technical, decision.style_mix_technical));
    require(approx_equal(plan.style_mix_spin, decision.style_mix_spin));
    require(approx_equal(plan.intent_weight_stabilize, decision.intent_weight_stabilize));
    require(approx_equal(plan.intent_weight_pressure, decision.intent_weight_pressure));
    require(approx_equal(plan.intent_weight_spintrap, decision.intent_weight_spintrap));
    require(approx_equal(plan.strike_commit_window_s, decision.strike_commit_window_s));
    require(approx_equal(plan.strike_min_make_prob, decision.strike_min_make_prob));
    require(approx_equal(plan.strike_velocity_target_abs, decision.strike_velocity_target_abs));

    const whacker::app::AiDecision round_trip = whacker::app::runtime_ai_decision_from_plan(plan);
    require(round_trip.valid);
    require(round_trip.inbound);
    require(round_trip.intent == whacker::app::AiIntent::SpinTrap);
    require(round_trip.candidate_id == decision.candidate_id);
    require(approx_equal(round_trip.intercept_time_s, plan.intercept_time_s));
    require(approx_equal(round_trip.intercept_y, plan.intercept_y));
    require(approx_equal(round_trip.contact_u, plan.contact_u));
    require(approx_equal(round_trip.strike_feedforward_vy, plan.strike_feedforward_vy));
    require(approx_equal(round_trip.pre_contact_target_y, plan.pre_contact_target_y));
    require(approx_equal(round_trip.post_contact_recover_y, plan.post_contact_recover_y));
    require(approx_equal(round_trip.confidence, plan.confidence));
    require(approx_equal(round_trip.score, plan.decision_score));
    require(round_trip.coarse_candidate_count == plan.coarse_candidate_count);
    require(round_trip.scored_candidate_count == plan.scored_candidate_count);
    require(round_trip.reachable_candidate_count == plan.reachable_candidate_count);
    require(round_trip.predicted_wall_bounces == plan.predicted_wall_bounces);
    require(approx_equal(round_trip.make_contact_probability, plan.make_contact_probability));
    require(approx_equal(round_trip.reach_slack, plan.reach_slack));
    require(round_trip.miss_risk_level == plan.miss_risk_level);
    require(approx_equal(round_trip.expected_impact_factor, plan.expected_impact_factor));
    require(approx_equal(round_trip.expected_spin_delta, plan.expected_spin_delta));
    require(approx_equal(round_trip.clean_contact_metric, plan.clean_contact_metric));
    require(approx_equal(round_trip.style_mix_power, plan.style_mix_power));
    require(approx_equal(round_trip.style_mix_technical, plan.style_mix_technical));
    require(approx_equal(round_trip.style_mix_spin, plan.style_mix_spin));
    require(approx_equal(round_trip.intent_weight_stabilize, plan.intent_weight_stabilize));
    require(approx_equal(round_trip.intent_weight_pressure, plan.intent_weight_pressure));
    require(approx_equal(round_trip.intent_weight_spintrap, plan.intent_weight_spintrap));
    require(approx_equal(round_trip.strike_commit_window_s, plan.strike_commit_window_s));
    require(approx_equal(round_trip.strike_min_make_prob, plan.strike_min_make_prob));
    require(approx_equal(round_trip.strike_velocity_target_abs, plan.strike_velocity_target_abs));
}

void test_invalid_decision_clears_runtime_plan() {
    whacker::app::RuntimeAiState ai_state {};
    ai_state.runtime_step_counter = 9;
    ai_state.plan.has_plan = true;
    ai_state.plan.plan_created_step = 7;
    ai_state.plan.candidate_id = 4;
    ai_state.plan.decision_score = 0.9f;

    whacker::app::AiDecision invalid_decision {};
    invalid_decision.valid = false;
    whacker::app::write_runtime_ai_plan_from_decision(ai_state, invalid_decision, 55ULL, false);

    require(!ai_state.plan.has_plan);
    require(ai_state.plan.plan_created_step == 0);
    require(ai_state.plan.valid_until_step == 0);
    require(ai_state.plan.replan_cooldown_steps == 0);
    require(approx_equal(ai_state.plan.intercept_time_s, 0.0f));
    require(ai_state.plan.candidate_id == -1);
    require(approx_equal(ai_state.plan.decision_score, 0.0f));
}

}  // namespace

int main() {
    test_runtime_plan_round_trip_preserves_mapped_fields();
    test_invalid_decision_clears_runtime_plan();
    return 0;
}

