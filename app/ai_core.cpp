#include "ai_core.hpp"

#include <cmath>
#include <cstdint>

#include "ai_candidates.hpp"
#include "ai_decision_templates.hpp"
#include "ai_execute.hpp"
#include "ai_frame.hpp"
#include "ai_predict.hpp"
#include "ai_profile.hpp"
#include "ai_reachability.hpp"
#include "ai_score.hpp"
#include "ai_seed.hpp"

namespace whacker::app {

namespace {

using ai_internal::actor_decision_to_world;
using ai_internal::keyed_noise_u01;
using ai_internal::make_actor_frame_simulation;
using ai_internal::make_noise_base;

}  // namespace

AiDecision plan_ai_decision(
    const whacker::sim::Simulation& simulation,
    const bool for_left_paddle,
    const RuntimeAiState& ai_state,
    const std::uint64_t decision_counter,
    const bool ambient_mode,
    const AiPlannerConfig& planner_config) {
    const whacker::sim::Simulation actor_simulation = make_actor_frame_simulation(simulation, for_left_paddle);
    const auto& state = actor_simulation.state();
    const auto& config = actor_simulation.config();
    const auto& self = state.left;
    const auto& opponent = state.right;

    const std::uint32_t base_seed = make_noise_base(state, ai_state.style, ai_state.skills);
    const float competence = ai_internal::competence_from_skills(ai_state.skills);
    const ai_internal::AiCapabilityProfile capability = ai_internal::capability_profile_for(competence);
    const ai_internal::StyleMix mix = ai_internal::style_mix_from_skills(ai_state);
    const ai_internal::IntentWeights weights = ai_internal::intent_weights_from_mix(mix);

    if (!ai_internal::moving_toward_actor_paddle(state)) {
        return actor_decision_to_world(
            ai_internal::make_recover_decision(actor_simulation, ai_state, competence, mix, weights),
            for_left_paddle);
    }

    whacker::sim::RallyState perceived_state = state;
    const float lag_s = capability.reaction_lag_s;
    if (lag_s > 0.0f) {
        perceived_state.ball.position.x -= perceived_state.ball.velocity.x * lag_s;
        perceived_state.ball.position.y = ai_internal::clampf(
            perceived_state.ball.position.y - (perceived_state.ball.velocity.y * lag_s),
            config.ball_radius,
            config.court_height - config.ball_radius);
    }

    const ai_internal::PredictorResult prediction = ai_internal::predict_intercept(
        perceived_state,
        config,
        planner_config.predictor_max_steps_inbound);

    if (!prediction.predicted) {
        return actor_decision_to_world(
            ai_internal::make_safe_intercept_decision(
                actor_simulation,
                ai_state,
                0.0f,
                0.0f,
                competence,
                ai_internal::clampf(
                    perceived_state.ball.position.y,
                    config.paddle_half_height,
                    config.court_height - config.paddle_half_height),
                mix,
                weights,
                capability),
            for_left_paddle);
    }

    const float jitter_u =
        (2.0f * keyed_noise_u01(base_seed, decision_counter, ai_internal::kPhaseJitter, 0, 0)) - 1.0f;
    const float intercept_noise = jitter_u * capability.intercept_jitter_amplitude;
    const float tracking_gain = ai_internal::clamp01f(
        ai_internal::lerpf(0.10f, 1.0f, competence) *
        ai_internal::lerpf(0.55f, 1.0f, prediction.confidence));
    const float noisy_intercept = prediction.intercept_y + intercept_noise;
    const float planned_intercept_y = ai_internal::clampf(
        ai_internal::lerpf(perceived_state.ball.position.y, noisy_intercept, tracking_gain),
        config.paddle_half_height,
        config.court_height - config.paddle_half_height);

    const ai_internal::ReachabilityEnvelope envelope = ai_internal::compute_reachability_envelope(
        self,
        config,
        prediction.t_hit,
        planner_config.reachability_max_steps,
        capability.speed_scale,
        capability.accel_scale);

    ai_internal::CandidateGenerationResult generated = ai_internal::generate_scored_candidates(
        state,
        config,
        self,
        opponent,
        ai_state,
        capability,
        mix,
        weights,
        prediction,
        envelope,
        planned_intercept_y);

    AiDecision out {};
    out.inbound = true;
    out.intercept_time_s = prediction.t_hit;
    out.intercept_y = planned_intercept_y;
    out.confidence = prediction.confidence;
    out.predicted_wall_bounces = prediction.wall_bounces;
    out.coarse_candidate_count = generated.candidate_count;

    if (generated.candidate_count <= 0) {
        return actor_decision_to_world(
            ai_internal::make_safe_intercept_decision(
                actor_simulation,
                ai_state,
                prediction.t_hit,
                prediction.confidence * 0.8f,
                competence,
                planned_intercept_y,
                mix,
                weights,
                capability),
            for_left_paddle);
    }

    const ai_internal::CandidateSelectionResult selection = ai_internal::select_best_candidate(
        generated.candidates,
        generated.candidate_count,
        planner_config.max_candidates,
        base_seed,
        decision_counter);

    if (!selection.has_winner) {
        return actor_decision_to_world(
            ai_internal::make_safe_intercept_decision(
                actor_simulation,
                ai_state,
                prediction.t_hit,
                prediction.confidence * 0.8f,
                competence,
                planned_intercept_y,
                mix,
                weights,
                capability),
            for_left_paddle);
    }

    out.scored_candidate_count = selection.scored_candidate_count;
    out.reachable_candidate_count = selection.reachable_candidate_count;

    const ai_internal::Candidate& winner = selection.winner;
    const float quant_step = capability.target_quantization_step;
    const float aim_noise_u =
        (2.0f * keyed_noise_u01(base_seed, decision_counter, ai_internal::kPhaseJitter, winner.id, 1)) - 1.0f;
    const float time_error = ai_internal::clamp01f(prediction.t_hit / 0.85f);
    const float confidence_error = 1.0f - prediction.confidence;
    const float wall_error = ai_internal::clamp01f(static_cast<float>(prediction.wall_bounces) / 3.0f);
    const float aim_error_amplitude =
        capability.intercept_jitter_amplitude *
        (0.42f + (0.88f * time_error) + (0.40f * confidence_error) + (0.30f * wall_error));
    const float raw_aim_error = aim_noise_u * aim_error_amplitude;
    const float tracking_error = winner.required_center_y - self.center_y;
    const float max_abs_error = std::max(5.0f, 0.95f * std::abs(tracking_error));
    const float bounded_error = ai_internal::clampf(raw_aim_error, -max_abs_error, max_abs_error);
    float aimed_center = winner.required_center_y + bounded_error;

    constexpr float kDirectionalEpsilon = 1.0f;
    if (tracking_error > kDirectionalEpsilon) {
        aimed_center = std::max(aimed_center, self.center_y + kDirectionalEpsilon);
    } else if (tracking_error < -kDirectionalEpsilon) {
        aimed_center = std::min(aimed_center, self.center_y - kDirectionalEpsilon);
    }

    const float pre_contact_target = ai_internal::clampf(
        std::round(aimed_center / quant_step) * quant_step,
        config.paddle_half_height,
        config.court_height - config.paddle_half_height);

    out.valid = true;
    out.intent = winner.intent;
    out.candidate_id = winner.id;
    out.contact_u = winner.contact_u;
    out.strike_feedforward_vy = winner.strike_vy;
    out.pre_contact_target_y = pre_contact_target;
    out.post_contact_recover_y = ai_internal::clampf(
        ai_internal::style_recover_lane_y_actor(config, ai_state.style, state.ball.spin),
        config.paddle_half_height,
        config.court_height - config.paddle_half_height);
    out.score = winner.score;

    out.make_term = winner.make_term;
    out.quality_term = winner.quality_term;
    out.style_term = winner.style_term;
    out.risk_term = winner.risk_term;
    out.motion_term = winner.motion_term;
    out.make_contact_probability = winner.make_term;
    out.reach_slack = winner.reach_slack;
    out.miss_risk_level = ai_internal::risk_level_from_term(winner.risk_term);

    out.expected_impact_factor = winner.impact_factor;
    out.expected_spin_delta = winner.spin_delta_estimate;
    out.clean_contact_metric = winner.clean_contact_metric;

    out.style_mix_power = mix.power;
    out.style_mix_technical = mix.technical;
    out.style_mix_spin = mix.spin;
    out.intent_weight_stabilize = weights.stabilize;
    out.intent_weight_pressure = weights.pressure;
    out.intent_weight_spintrap = weights.spintrap;

    out.strike_commit_window_s = 0.0f;
    out.strike_min_make_prob = 1.0f;
    out.strike_velocity_target_abs = std::abs(winner.strike_vy);
    if (winner.intent == AiIntent::Pressure) {
        out.strike_commit_window_s = ai_internal::clampf(0.06f + (0.05f * competence), 0.06f, 0.11f);
        out.strike_min_make_prob = ai_internal::clampf(
            0.44f - (0.08f * competence) + (0.08f * winner.risk_term),
            0.32f,
            0.52f);
    } else if (winner.intent == AiIntent::SpinTrap) {
        out.strike_commit_window_s = ai_internal::clampf(
            0.09f + (0.10f * generated.spin_skill) + (0.08f * mix.spin),
            0.09f,
            0.28f);
        out.strike_min_make_prob = ai_internal::clampf(
            0.20f - (0.10f * generated.spin_skill) - (0.06f * mix.spin) + (0.10f * winner.risk_term),
            0.08f,
            0.38f);
    }

    out.valid_steps = ai_internal::clampi(
        static_cast<int>(std::lround(
            18.0f +
            (36.0f * (1.0f - competence)) +
            (18.0f * (1.0f - prediction.confidence)))),
        12,
        90);
    const float cooldown_noise =
        keyed_noise_u01(base_seed, decision_counter, ai_internal::kPhaseCooldown, winner.id, 0);
    out.cooldown_steps = ai_internal::clampi(
        static_cast<int>(std::lround(3.0f + (7.0f * (1.0f - competence)) + (2.0f * cooldown_noise))),
        2,
        14);

    if (ambient_mode) {
        out.pre_contact_target_y = ai_internal::clampf(
            self.center_y + (0.55f * (out.pre_contact_target_y - self.center_y)),
            config.paddle_half_height,
            config.court_height - config.paddle_half_height);
        out.strike_feedforward_vy = 0.0f;
        out.strike_commit_window_s = 0.0f;
        out.strike_min_make_prob = 1.0f;
        out.strike_velocity_target_abs = 0.0f;
    }

    return actor_decision_to_world(out, for_left_paddle);
}

void apply_ai_decision(
    whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config,
    const AiDecision& decision,
    const bool inbound,
    const float intercept_time_s,
    const bool ambient_mode) {
    ai_internal::apply_ai_decision_to_paddle(
        paddle,
        config,
        decision,
        inbound,
        intercept_time_s,
        ambient_mode);
}

uint64_t compute_ai_replan_signature(const whacker::sim::RallyState& state, const bool for_left_paddle) {
    return ai_internal::compute_state_signature_from_rally_state(state, for_left_paddle);
}

uint64_t compute_ai_state_signature(const whacker::sim::Simulation& simulation, const bool for_left_paddle) {
    return ai_internal::compute_state_signature_from_simulation(simulation, for_left_paddle);
}

}  // namespace whacker::app
