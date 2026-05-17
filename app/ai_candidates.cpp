#include "ai_candidates.hpp"

#include "ai_profile.hpp"
#include "sim/math.hpp"

namespace whacker::app::ai_internal {

namespace {

float risk_term_from_ratio_and_u(const float speed_ratio, const float abs_u, const float edge_skill) {
    const float near_limit = clamp01f((speed_ratio - 0.85f) / 0.25f);
    const float safe_u = 0.40f + (0.45f * edge_skill);
    const float edge_risk = clamp01f((abs_u - safe_u) / 0.35f);
    return clamp01f((0.65f * near_limit) + (0.35f * edge_risk));
}

}  // namespace

CandidateGenerationResult generate_scored_candidates(
    const whacker::sim::RallyState& state,
    const whacker::sim::SimulationConfig& config,
    const whacker::sim::PaddleState& self,
    const whacker::sim::PaddleState& opponent,
    const RuntimeAiState& ai_state,
    const AiCapabilityProfile& capability,
    const StyleMix& mix,
    const IntentWeights& weights,
    const PredictorResult& prediction,
    const ReachabilityEnvelope& envelope,
    const float planned_intercept_y) {
    CandidateGenerationResult out {};

    static constexpr std::array<float, 7> kContactSet {
        -0.72f,
        -0.42f,
        -0.18f,
        0.0f,
        0.18f,
        0.42f,
        0.72f,
    };

    int next_candidate_id = 0;

    const float edge_skill = clamp01f(ai_state.skills.edge);
    const float power_skill = clamp01f(ai_state.skills.power);
    const float spin_skill = clamp01f(ai_state.skills.spin_inject);
    const float vmax = std::max(config.paddle_max_speed * capability.speed_scale, 1.0e-3f);
    const float strike_vmax = vmax;
    const float competence_make_floor = capability.makeability_scale;

    const float u_cap = clampf(0.22f + (0.58f * edge_skill), 0.22f, 0.82f);
    const float pressure_speed = strike_vmax * (0.14f + (0.40f * power_skill) + (0.20f * mix.power));
    const float spin_speed = strike_vmax * (0.16f + (0.46f * spin_skill) + (0.22f * mix.spin));

    float pressure_dir = signf((0.5f * config.court_height) - planned_intercept_y);
    if (pressure_dir == 0.0f) {
        pressure_dir = signf(opponent.center_y - planned_intercept_y);
    }
    if (pressure_dir == 0.0f) {
        pressure_dir = 1.0f;
    }

    auto push_candidate = [&](const AiIntent intent, const float u, const float strike_vy) {
        if (out.candidate_count >= static_cast<int>(out.candidates.size())) {
            return;
        }

        const float required_center = planned_intercept_y - (u * config.paddle_half_height);
        const bool reachable =
            (required_center >= (envelope.min_center_y - kMinReachEpsilon)) &&
            (required_center <= (envelope.max_center_y + kMinReachEpsilon));
        if (!reachable) {
            return;
        }

        const float reach_slack = std::min(
            required_center - envelope.min_center_y,
            envelope.max_center_y - required_center);
        const float required_speed =
            std::abs(required_center - self.center_y) / std::max(prediction.t_hit, whacker::sim::kFixedDt);
        const float required_speed_ratio = required_speed / vmax;

        Candidate c {};
        c.id = next_candidate_id;
        ++next_candidate_id;
        c.intent = intent;
        c.contact_u = u;
        c.strike_vy = strike_vy;
        c.required_center_y = required_center;
        c.required_speed_ratio = required_speed_ratio;
        c.reach_slack = reach_slack;

        const float slack_norm = clamp01f(
            (reach_slack + 0.75f) /
            (0.45f * config.paddle_half_height + 0.75f));
        const float speed_margin = clamp01f(1.0f - (required_speed_ratio / 1.05f));
        c.make_term = clamp01f((0.60f * slack_norm) + (0.40f * speed_margin));
        c.make_term = clamp01f(c.make_term * competence_make_floor);
        if (required_speed_ratio > 1.10f) {
            c.make_term *= 0.50f;
        }

        c.impact_factor = whacker::sim::impact_power_factor(c.strike_vy, vmax);
        const float placement = clamp01f(std::abs(u));
        const float vy_dampen =
            clamp01f(1.0f - (std::abs(prediction.intercept_vy) / std::max(vmax, 1.0e-3f)));
        const float opponent_offset = std::abs(opponent.center_y - c.required_center_y) /
            std::max(config.court_height, 1.0e-3f);
        c.quality_term = clamp01f(
            (0.42f * c.impact_factor) +
            (0.22f * placement) +
            (0.18f * vy_dampen) +
            (0.18f * placement * clamp01f(opponent_offset)));

        c.spin_delta_estimate = whacker::sim::contact_spin_delta(
            config,
            c.strike_vy,
            spin_skill,
            1.0f);
        const float spin_signal = clamp01f(
            std::abs(c.spin_delta_estimate) / std::max(0.35f * config.spin_max, 1.0e-3f));
        const float alignment = (std::abs(state.ball.spin) <= 1.0e-6f || std::abs(c.strike_vy) <= 1.0e-6f)
            ? 0.50f
            : ((signf(state.ball.spin) != signf(c.strike_vy)) ? 1.0f : 0.0f);

        const float power_pref = clamp01f((0.72f * c.impact_factor) + (0.28f * placement));
        const float technical_pref = clamp01f((0.58f * placement * edge_skill) + (0.42f * c.make_term));
        const float spin_pref = clamp01f((0.70f * spin_signal) + (0.30f * alignment));
        c.style_term = clamp01f(
            (mix.power * power_pref) +
            (mix.technical * technical_pref) +
            (mix.spin * spin_pref));

        c.risk_term = risk_term_from_ratio_and_u(required_speed_ratio, std::abs(u), edge_skill);
        c.motion_term = clamp01f(
            std::abs(required_center - self.center_y) /
            std::max(0.45f * config.court_height, 1.0e-3f));

        const float style_bias = 0.12f * (intent_weight_for(weights, intent) - (1.0f / 3.0f));
        c.score =
            (0.50f * c.make_term) +
            (0.24f * c.quality_term) +
            (0.22f * c.style_term) -
            (0.18f * c.risk_term) -
            (0.06f * c.motion_term) +
            style_bias;
        if (c.make_term < 0.22f) {
            c.score -= 0.06f;
        }

        c.clean_contact_metric = c.make_term;
        c.cheap_score = c.make_term + (0.25f * c.style_term) - (0.18f * c.risk_term);

        out.candidates[static_cast<std::size_t>(out.candidate_count)] = c;
        ++out.candidate_count;
    };

    for (float u : kContactSet) {
        const float abs_u = std::abs(u);
        if (abs_u > u_cap) {
            continue;
        }

        push_candidate(AiIntent::Stabilize, u, 0.0f);
        if (abs_u >= 0.18f) {
            push_candidate(AiIntent::Pressure, u, pressure_dir * pressure_speed);
            push_candidate(AiIntent::SpinTrap, u, -spin_speed);
            push_candidate(AiIntent::SpinTrap, u, spin_speed);
        }
    }

    out.vmax = vmax;
    out.spin_skill = spin_skill;
    return out;
}

}  // namespace whacker::app::ai_internal
