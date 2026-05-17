#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "ai_core.hpp"

namespace {

bool approx_equal(const float a, const float b, const float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

void require_impl(const bool condition, const char* expression, const int line) {
    if (!condition) {
        std::fprintf(stderr, "ai_core_smoke assertion failed at line %d: %s\n", line, expression);
        std::abort();
    }
}

#define REQUIRE(condition) require_impl((condition), #condition, __LINE__)

whacker::app::RuntimeAiState make_ai_state(
    const whacker::app::AiStyle style,
    const whacker::progression::SkillState& skills) {
    whacker::app::RuntimeAiState state {};
    state.initialized = true;
    state.style = style;
    state.skills = skills;
    return state;
}

void setup_common_inbound_left(whacker::sim::Simulation& simulation) {
    auto& state = simulation.mutable_state();
    state.ball.position.x = 560.0f;
    state.ball.position.y = 132.0f;
    state.ball.velocity.x = -320.0f;
    state.ball.velocity.y = 74.0f;
    state.ball.spin = 2.0f;
    state.ball.speed_scalar = 1.10f;
    state.left.center_y = 282.0f;
    state.left.velocity_y = -48.0f;
    state.right.center_y = 78.0f;
    state.right.velocity_y = 18.0f;
}

void test_same_seed_same_decision() {
    whacker::sim::Simulation simulation {};
    setup_common_inbound_left(simulation);

    const auto skills = whacker::progression::SkillState {.edge = 0.42f, .power = 0.30f, .spin_inject = 0.28f};
    const whacker::app::RuntimeAiState ai_state = make_ai_state(whacker::app::AiStyle::Balanced, skills);

    const whacker::app::AiDecision a =
        whacker::app::plan_ai_decision(simulation, true, ai_state, 42ULL, false);
    const whacker::app::AiDecision b =
        whacker::app::plan_ai_decision(simulation, true, ai_state, 42ULL, false);

    REQUIRE(a.valid == b.valid);
    REQUIRE(a.inbound == b.inbound);
    REQUIRE(a.intent == b.intent);
    REQUIRE(a.candidate_id == b.candidate_id);
    REQUIRE(approx_equal(a.pre_contact_target_y, b.pre_contact_target_y));
    REQUIRE(approx_equal(a.post_contact_recover_y, b.post_contact_recover_y));
    REQUIRE(approx_equal(a.score, b.score));
    REQUIRE(approx_equal(a.expected_spin_delta, b.expected_spin_delta));
    REQUIRE(approx_equal(a.expected_impact_factor, b.expected_impact_factor));
}

void test_decision_outputs_are_bounded() {
    whacker::sim::Simulation simulation {};
    setup_common_inbound_left(simulation);

    const auto skills = whacker::progression::SkillState {.edge = 0.12f, .power = 0.12f, .spin_inject = 0.12f};
    const whacker::app::AiDecision decision = whacker::app::plan_ai_decision(
        simulation,
        true,
        make_ai_state(whacker::app::AiStyle::Balanced, skills),
        7ULL,
        false);
    const auto& config = simulation.config();

    REQUIRE(decision.valid);
    REQUIRE(decision.coarse_candidate_count >= decision.scored_candidate_count);
    REQUIRE(decision.scored_candidate_count <= 24);
    REQUIRE(decision.reachable_candidate_count <= decision.scored_candidate_count);
    REQUIRE(decision.pre_contact_target_y >= config.paddle_half_height);
    REQUIRE(decision.pre_contact_target_y <= (config.court_height - config.paddle_half_height));
    REQUIRE(decision.post_contact_recover_y >= config.paddle_half_height);
    REQUIRE(decision.post_contact_recover_y <= (config.court_height - config.paddle_half_height));
    REQUIRE(decision.make_contact_probability >= 0.0f);
    REQUIRE(decision.make_contact_probability <= 1.0f);
}

void test_outbound_uses_recover_mode() {
    whacker::sim::Simulation simulation {};
    setup_common_inbound_left(simulation);
    simulation.mutable_state().ball.velocity.x = 280.0f;

    const auto skills = whacker::progression::SkillState {.edge = 0.34f, .power = 0.33f, .spin_inject = 0.33f};
    const whacker::app::AiDecision decision = whacker::app::plan_ai_decision(
        simulation,
        true,
        make_ai_state(whacker::app::AiStyle::Power, skills),
        11ULL,
        false);
    const auto& config = simulation.config();

    REQUIRE(decision.valid);
    REQUIRE(!decision.inbound);
    REQUIRE(decision.intent == whacker::app::AiIntent::Stabilize);
    REQUIRE(decision.strike_feedforward_vy == 0.0f);
    REQUIRE(decision.post_contact_recover_y >= config.paddle_half_height);
    REQUIRE(decision.post_contact_recover_y <= (config.court_height - config.paddle_half_height));
}

void test_simulation_signature_tracks_motion() {
    whacker::sim::Simulation simulation {};
    setup_common_inbound_left(simulation);

    const std::uint64_t before = whacker::app::compute_ai_state_signature(simulation, true);
    simulation.mutable_state().ball.position.x -= 144.0f;
    simulation.mutable_state().ball.position.y += 32.0f;
    const std::uint64_t after = whacker::app::compute_ai_state_signature(simulation, true);

    REQUIRE(before != after);
}

void test_contact_gate_blocks_misaligned_strike() {
    whacker::sim::Simulation simulation {};
    const auto& config = simulation.config();
    auto paddle = simulation.state().left;
    paddle.center_y = 80.0f;

    whacker::app::AiDecision decision {};
    decision.valid = true;
    decision.intent = whacker::app::AiIntent::SpinTrap;
    decision.pre_contact_target_y = 220.0f;
    decision.post_contact_recover_y = 180.0f;
    decision.strike_feedforward_vy = 170.0f;
    decision.strike_commit_window_s = 0.18f;
    decision.strike_min_make_prob = 0.20f;
    decision.strike_velocity_target_abs = 170.0f;
    decision.make_contact_probability = 0.95f;
    decision.miss_risk_level = 0;
    decision.style_mix_technical = 0.1f;

    whacker::app::apply_ai_decision(
        paddle,
        config,
        decision,
        true,
        0.04f,
        false);

    REQUIRE(std::fabs(paddle.feedforward_velocity_y) <= 1.0e-5f);
}

void test_contact_gate_allows_aligned_strike() {
    whacker::sim::Simulation simulation {};
    const auto& config = simulation.config();
    auto paddle = simulation.state().left;

    whacker::app::AiDecision decision {};
    decision.valid = true;
    decision.intent = whacker::app::AiIntent::SpinTrap;
    decision.pre_contact_target_y = 180.0f;
    decision.post_contact_recover_y = 180.0f;
    decision.strike_feedforward_vy = 160.0f;
    decision.strike_commit_window_s = 0.18f;
    decision.strike_min_make_prob = 0.20f;
    decision.strike_velocity_target_abs = 160.0f;
    decision.make_contact_probability = 0.95f;
    decision.miss_risk_level = 0;
    decision.style_mix_technical = 0.2f;

    paddle.center_y = decision.pre_contact_target_y;
    whacker::app::apply_ai_decision(
        paddle,
        config,
        decision,
        true,
        0.04f,
        false);

    REQUIRE(std::fabs(paddle.feedforward_velocity_y) > 1.0e-5f);
}

void test_ambient_tracks_inbound_then_recovers_to_lane() {
    whacker::sim::Simulation simulation {};
    setup_common_inbound_left(simulation);
    const auto& config = simulation.config();

    const auto skills = whacker::progression::SkillState {.edge = 0.10f, .power = 0.10f, .spin_inject = 0.40f};
    const auto ai_state = make_ai_state(whacker::app::AiStyle::Spin, skills);
    const whacker::app::AiDecision inbound = whacker::app::plan_ai_decision(
        simulation,
        true,
        ai_state,
        17ULL,
        true);
    simulation.mutable_state().ball.velocity.x = 280.0f;
    const whacker::app::AiDecision outbound = whacker::app::plan_ai_decision(
        simulation,
        true,
        ai_state,
        18ULL,
        true);

    REQUIRE(inbound.valid);
    REQUIRE(outbound.valid);
    REQUIRE(inbound.inbound);
    REQUIRE(!outbound.inbound);
    REQUIRE(inbound.pre_contact_target_y >= config.paddle_half_height);
    REQUIRE(inbound.pre_contact_target_y <= (config.court_height - config.paddle_half_height));
    REQUIRE(outbound.post_contact_recover_y >= config.paddle_half_height);
    REQUIRE(outbound.post_contact_recover_y <= (config.court_height - config.paddle_half_height));
    REQUIRE(std::fabs(inbound.pre_contact_target_y - outbound.post_contact_recover_y) > 2.0f);
}

}  // namespace

int main() {
    test_same_seed_same_decision();
    test_decision_outputs_are_bounded();
    test_outbound_uses_recover_mode();
    test_simulation_signature_tracks_motion();
    test_contact_gate_blocks_misaligned_strike();
    test_contact_gate_allows_aligned_strike();
    test_ambient_tracks_inbound_then_recovers_to_lane();
    return 0;
}
