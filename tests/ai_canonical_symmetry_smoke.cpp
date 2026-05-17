#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "ai_core.hpp"

namespace {

namespace app = whacker::app;
namespace sim = whacker::sim;

void require_impl(const bool condition, const char* expression, const int line) {
    if (!condition) {
        std::fprintf(stderr, "ai_canonical_symmetry_smoke assertion failed at line %d: %s\n", line, expression);
        std::abort();
    }
}

#define REQUIRE(condition) require_impl((condition), #condition, __LINE__)

void require_near_named(const float a, const float b, const float epsilon, const char* label) {
    if (std::fabs(a - b) <= epsilon) {
        return;
    }
    std::fprintf(
        stderr,
        "ai_canonical_symmetry_smoke near-check failed: %s (a=%.6f b=%.6f eps=%.6f)\n",
        label,
        static_cast<double>(a),
        static_cast<double>(b),
        static_cast<double>(epsilon));
    std::abort();
}

#define REQUIRE_NEAR(a, b, eps) require_near_named((a), (b), (eps), #a " vs " #b)

sim::RallyState mirror_state_x(const sim::SimulationConfig& config, const sim::RallyState& source) {
    sim::RallyState out = source;
    out.ball.position.x = config.court_width - source.ball.position.x;
    out.ball.position.y = source.ball.position.y;
    out.ball.velocity.x = -source.ball.velocity.x;
    out.ball.velocity.y = source.ball.velocity.y;
    out.ball.spin = -source.ball.spin;

    out.left = source.right;
    out.right = source.left;
    out.left_score = source.right_score;
    out.right_score = source.left_score;
    return out;
}

void seed_fixture(sim::Simulation& simulation, const int fixture_id) {
    auto& state = simulation.mutable_state();
    const auto& config = simulation.config();

    const float width = config.court_width;
    const float height = config.court_height;

    const float fx = static_cast<float>((fixture_id * 73) % 1000) / 1000.0f;
    const float fy = static_cast<float>((fixture_id * 211) % 1000) / 1000.0f;
    const float fv = static_cast<float>((fixture_id * 307) % 1000) / 1000.0f;
    const float fs = static_cast<float>((fixture_id * 419) % 1000) / 1000.0f;

    state.ball.position.x = (0.20f + (0.60f * fx)) * width;
    state.ball.position.y = (0.15f + (0.70f * fy)) * height;
    const float base_speed = config.ball_base_speed;
    const float vx_mag = base_speed * (0.65f + (0.70f * fv));
    state.ball.velocity.x = (fixture_id % 2 == 0) ? -vx_mag : vx_mag;
    state.ball.velocity.y = base_speed * ((fs * 1.4f) - 0.7f);
    state.ball.spin = ((static_cast<float>((fixture_id * 137) % 1000) / 1000.0f) * 7.0f) - 3.5f;
    state.ball.speed_scalar = 0.90f + (0.35f * static_cast<float>((fixture_id * 61) % 1000) / 1000.0f);

    state.left.center_y = (0.20f + (0.60f * static_cast<float>((fixture_id * 89) % 1000) / 1000.0f)) * height;
    state.right.center_y = (0.20f + (0.60f * static_cast<float>((fixture_id * 149) % 1000) / 1000.0f)) * height;
    state.left.velocity_y = ((static_cast<float>((fixture_id * 173) % 1000) / 1000.0f) * 140.0f) - 70.0f;
    state.right.velocity_y = ((static_cast<float>((fixture_id * 191) % 1000) / 1000.0f) * 140.0f) - 70.0f;
    state.left_score = fixture_id % 6;
    state.right_score = (fixture_id * 3) % 6;
    state.rally_hits = static_cast<std::uint64_t>((fixture_id * 11) % 29);
}

void require_mirrored_decisions(const app::AiDecision& left, const app::AiDecision& right) {
    REQUIRE(left.valid == right.valid);
    if (!left.valid) {
        return;
    }

    REQUIRE(left.inbound == right.inbound);
    REQUIRE(left.intent == right.intent);
    REQUIRE(left.candidate_id == right.candidate_id);
    REQUIRE_NEAR(left.intercept_time_s, right.intercept_time_s, 5.0e-3f);
    REQUIRE_NEAR(left.intercept_y, right.intercept_y, 0.5f);
    REQUIRE_NEAR(left.contact_u, right.contact_u, 5.0e-2f);
    REQUIRE_NEAR(left.pre_contact_target_y, right.pre_contact_target_y, 1.0f);
    REQUIRE_NEAR(left.post_contact_recover_y, right.post_contact_recover_y, 1.0f);
    REQUIRE_NEAR(left.strike_feedforward_vy, -right.strike_feedforward_vy, 1.0f);
    REQUIRE_NEAR(left.confidence, right.confidence, 1.0e-2f);
    REQUIRE_NEAR(left.score, right.score, 5.0e-2f);
    REQUIRE(left.coarse_candidate_count == right.coarse_candidate_count);
    REQUIRE(left.scored_candidate_count == right.scored_candidate_count);
    REQUIRE(left.reachable_candidate_count == right.reachable_candidate_count);
    REQUIRE(left.predicted_wall_bounces == right.predicted_wall_bounces);
    REQUIRE_NEAR(left.make_contact_probability, right.make_contact_probability, 2.0e-2f);
    REQUIRE_NEAR(left.reach_slack, right.reach_slack, 2.0f);
    REQUIRE(left.miss_risk_level == right.miss_risk_level);
    REQUIRE_NEAR(left.expected_impact_factor, right.expected_impact_factor, 5.0e-2f);
    REQUIRE_NEAR(left.expected_spin_delta, -right.expected_spin_delta, 3.0e-1f);
    REQUIRE_NEAR(left.clean_contact_metric, right.clean_contact_metric, 2.0e-2f);
}

app::RuntimeAiState make_ai_state(const whacker::progression::SkillState& skills) {
    app::RuntimeAiState ai {};
    ai.initialized = true;
    ai.style = app::AiStyle::Balanced;
    ai.skills = skills;
    return ai;
}

void test_signature_and_decision_symmetry() {
    constexpr whacker::progression::SkillState kProfiles[] = {
        {.edge = 0.10f, .power = 0.10f, .spin_inject = 0.10f},
        {.edge = 0.34f, .power = 0.33f, .spin_inject = 0.33f},
        {.edge = 0.57f, .power = 0.57f, .spin_inject = 0.56f},
    };

    for (int fixture = 0; fixture < 8; ++fixture) {
        sim::Simulation sim_a {};
        sim::Simulation sim_b {};
        seed_fixture(sim_a, fixture);
        sim_b.mutable_state() = mirror_state_x(sim_b.config(), sim_a.state());

        for (const auto& skills : kProfiles) {
            const app::RuntimeAiState left_ai = make_ai_state(skills);
            const app::RuntimeAiState right_ai = make_ai_state(skills);

            for (int frame = 0; frame < 120; ++frame) {
                if (frame > 0) {
                    sim_b.mutable_state() = mirror_state_x(sim_b.config(), sim_a.state());
                }

                const std::uint64_t signature_left = app::compute_ai_state_signature(sim_a, true);
                const std::uint64_t signature_right = app::compute_ai_state_signature(sim_b, false);
                REQUIRE(signature_left == signature_right);

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
                require_mirrored_decisions(left_decision, right_decision);

                (void)sim_a.step(sim::kFixedDt);
            }
        }
    }
}

}  // namespace

int main() {
    test_signature_and_decision_symmetry();
    return 0;
}
