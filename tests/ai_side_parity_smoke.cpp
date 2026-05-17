#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "ai_core.hpp"

namespace {

namespace app = whacker::app;
namespace sim = whacker::sim;

float signf(const float value) {
    if (value > 0.0f) {
        return 1.0f;
    }
    if (value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

void require_impl(const bool condition, const char* expression, const int line) {
    if (!condition) {
        std::fprintf(stderr, "ai_side_parity_smoke assertion failed at line %d: %s\n", line, expression);
        std::abort();
    }
}

#define REQUIRE(condition) require_impl((condition), #condition, __LINE__)

void require_near(const float a, const float b, const float epsilon) {
    REQUIRE(std::fabs(a - b) <= epsilon);
}

app::RuntimeAiState make_ai_state(
    const app::AiStyle style,
    const whacker::progression::SkillState& skills) {
    app::RuntimeAiState state {};
    state.initialized = true;
    state.style = style;
    state.skills = skills;
    return state;
}

void mirror_state_into(
    const sim::SimulationConfig& config,
    const sim::RallyState& source,
    sim::RallyState& dest) {
    dest = source;
    dest.ball.position.x = config.court_width - source.ball.position.x;
    dest.ball.position.y = source.ball.position.y;
    dest.ball.velocity.x = -source.ball.velocity.x;
    dest.ball.velocity.y = source.ball.velocity.y;
    dest.ball.spin = -source.ball.spin;

    dest.left = source.right;
    dest.right = source.left;
    dest.left_score = source.right_score;
    dest.right_score = source.left_score;
}

void seed_state(sim::Simulation& simulation) {
    auto& state = simulation.mutable_state();
    state.ball.position.x = 430.0f;
    state.ball.position.y = 168.0f;
    state.ball.velocity.x = -292.0f;
    state.ball.velocity.y = 74.0f;
    state.ball.spin = 1.8f;
    state.ball.speed_scalar = 1.12f;
    state.left.center_y = 264.0f;
    state.left.velocity_y = -38.0f;
    state.right.center_y = 118.0f;
    state.right.velocity_y = 26.0f;
    state.left_score = 3;
    state.right_score = 5;
    state.rally_hits = 7;
}

void require_mirror_equivalent(const app::AiDecision& left, const app::AiDecision& right) {
    REQUIRE(left.valid == right.valid);
    if (!left.valid) {
        return;
    }

    REQUIRE(left.inbound == right.inbound);
    require_near(left.intercept_time_s, right.intercept_time_s, 1.0e-3f);
    require_near(left.intercept_y, right.intercept_y, 1.0e-3f);
    REQUIRE(left.intent == right.intent);
    REQUIRE(left.candidate_id == right.candidate_id);
    require_near(left.contact_u, right.contact_u, 1.0e-3f);
    require_near(left.pre_contact_target_y, right.pre_contact_target_y, 1.0e-3f);
    require_near(left.post_contact_recover_y, right.post_contact_recover_y, 1.0e-3f);
    require_near(left.strike_feedforward_vy, -right.strike_feedforward_vy, 1.0e-3f);
    require_near(left.confidence, right.confidence, 1.0e-3f);
    require_near(left.score, right.score, 1.0e-3f);
    REQUIRE(left.coarse_candidate_count == right.coarse_candidate_count);
    REQUIRE(left.scored_candidate_count == right.scored_candidate_count);
    REQUIRE(left.reachable_candidate_count == right.reachable_candidate_count);
    REQUIRE(left.predicted_wall_bounces == right.predicted_wall_bounces);
    require_near(left.make_contact_probability, right.make_contact_probability, 1.0e-3f);
    require_near(left.reach_slack, right.reach_slack, 1.0e-3f);
    REQUIRE(left.miss_risk_level == right.miss_risk_level);
    require_near(left.expected_impact_factor, right.expected_impact_factor, 1.0e-3f);
    require_near(left.expected_spin_delta, -right.expected_spin_delta, 1.0e-3f);
    require_near(left.clean_contact_metric, right.clean_contact_metric, 1.0e-3f);
    require_near(left.style_mix_power, right.style_mix_power, 1.0e-3f);
    require_near(left.style_mix_technical, right.style_mix_technical, 1.0e-3f);
    require_near(left.style_mix_spin, right.style_mix_spin, 1.0e-3f);
    require_near(left.intent_weight_stabilize, right.intent_weight_stabilize, 1.0e-3f);
    require_near(left.intent_weight_pressure, right.intent_weight_pressure, 1.0e-3f);
    require_near(left.intent_weight_spintrap, right.intent_weight_spintrap, 1.0e-3f);
    require_near(left.strike_commit_window_s, right.strike_commit_window_s, 1.0e-3f);
    require_near(left.strike_min_make_prob, right.strike_min_make_prob, 1.0e-3f);
    require_near(left.strike_velocity_target_abs, right.strike_velocity_target_abs, 1.0e-3f);
}

struct TrackingMetrics {
    int tracking_frames = 0;
    int anti_tracking_frames = 0;
};

void require_decision_bounds(
    const app::AiDecision& decision,
    const sim::SimulationConfig& config) {
    if (!decision.valid) {
        return;
    }

    const float min_y = config.paddle_half_height;
    const float max_y = config.court_height - config.paddle_half_height;
    REQUIRE(decision.pre_contact_target_y >= (min_y - 1.0e-4f));
    REQUIRE(decision.pre_contact_target_y <= (max_y + 1.0e-4f));
    REQUIRE(decision.post_contact_recover_y >= (min_y - 1.0e-4f));
    REQUIRE(decision.post_contact_recover_y <= (max_y + 1.0e-4f));
    REQUIRE(std::abs(decision.strike_feedforward_vy) <= (config.paddle_max_speed + 1.0e-4f));
}

void accumulate_tracking_quality(
    const app::AiDecision& decision,
    const sim::PaddleState& paddle,
    TrackingMetrics& metrics) {
    if (!decision.valid || !decision.inbound) {
        return;
    }

    const float error = decision.intercept_y - paddle.center_y;
    const float command = decision.pre_contact_target_y - paddle.center_y;
    if (std::fabs(error) < 12.0f || std::fabs(command) < 2.0f) {
        return;
    }

    ++metrics.tracking_frames;
    if (signf(error) != signf(command)) {
        ++metrics.anti_tracking_frames;
    }
}

void test_step_mirror_decision_parity() {
    sim::Simulation sim_a {};
    sim::Simulation sim_b {};
    seed_state(sim_a);
    mirror_state_into(sim_a.config(), sim_a.state(), sim_b.mutable_state());

    const auto skills = whacker::progression::SkillState {.edge = 0.34f, .power = 0.33f, .spin_inject = 0.33f};
    const app::RuntimeAiState left_ai = make_ai_state(app::AiStyle::Balanced, skills);
    const app::RuntimeAiState right_ai = make_ai_state(app::AiStyle::Balanced, skills);
    TrackingMetrics left_metrics {};
    TrackingMetrics right_metrics {};

    constexpr int kFrames = 600;
    for (int frame = 0; frame < kFrames; ++frame) {
        if (frame > 0) {
            mirror_state_into(sim_a.config(), sim_a.state(), sim_b.mutable_state());
        }

        const app::AiDecision left_decision = app::plan_ai_decision(
            sim_a,
            true,
            left_ai,
            static_cast<std::uint64_t>(frame),
            false);
        const app::AiDecision right_decision = app::plan_ai_decision(
            sim_b,
            false,
            right_ai,
            static_cast<std::uint64_t>(frame),
            false);

        require_mirror_equivalent(left_decision, right_decision);
        require_decision_bounds(left_decision, sim_a.config());
        require_decision_bounds(right_decision, sim_b.config());
        accumulate_tracking_quality(left_decision, sim_a.state().left, left_metrics);
        accumulate_tracking_quality(right_decision, sim_b.state().right, right_metrics);
        (void)sim_a.step(sim::kFixedDt);
    }

    const float left_ratio = left_metrics.tracking_frames > 0
        ? static_cast<float>(left_metrics.anti_tracking_frames) / static_cast<float>(left_metrics.tracking_frames)
        : 0.0f;
    const float right_ratio = right_metrics.tracking_frames > 0
        ? static_cast<float>(right_metrics.anti_tracking_frames) / static_cast<float>(right_metrics.tracking_frames)
        : 0.0f;
    REQUIRE(left_ratio <= 0.30f);
    REQUIRE(right_ratio <= 0.30f);
    REQUIRE(std::fabs(left_ratio - right_ratio) <= 0.10f);
}

}  // namespace

int main() {
    test_step_mirror_decision_parity();
    return 0;
}
