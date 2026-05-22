#include <cmath>
#include <cstdlib>

#include "ai_core.hpp"
#include "play_control.hpp"

namespace {

bool approx_equal(const float a, const float b, const float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

void require_in_range(const float value, const float lo, const float hi) {
    require(value >= lo);
    require(value <= hi);
}

void setup_inbound_left_state(whacker::sim::Simulation& simulation) {
    auto& state = simulation.mutable_state();
    state.ball.position.x = 520.0f;
    state.ball.position.y = 108.0f;
    state.ball.velocity.x = -340.0f;
    state.ball.velocity.y = 88.0f;
    state.ball.spin = 1.9f;
    state.ball.speed_scalar = 1.16f;
    state.left.center_y = 292.0f;
    state.left.velocity_y = -55.0f;
    state.right.center_y = 82.0f;
    state.right.velocity_y = 35.0f;
}

void configure_ai_match_options(whacker::app::MatchOptions& options) {
    options.left_mode = whacker::app::PaddleMode::AI;
    options.right_mode = whacker::app::PaddleMode::AI;
    options.left_paddle_skills = {.edge = 0.56f, .power = 0.68f, .spin_inject = 0.40f};
    options.right_paddle_skills = {.edge = 0.44f, .power = 0.34f, .spin_inject = 0.74f};
}

void test_ai_mode_produces_bounded_targets_and_plan_state() {
    whacker::sim::Simulation simulation {};
    setup_inbound_left_state(simulation);
    const auto& config = simulation.config();

    whacker::app::MatchOptions options {};
    configure_ai_match_options(options);

    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};

    whacker::app::update_targets_for_play(
        simulation,
        options,
        left_ai_state,
        right_ai_state,
        whacker::sim::kFixedDt,
        0.0f,
        0.0f,
        nullptr);

    const auto& state = simulation.state();
    const float min_y = config.paddle_half_height;
    const float max_y = config.court_height - config.paddle_half_height;

    require(left_ai_state.plan.has_plan);
    require(right_ai_state.plan.has_plan);
    require(left_ai_state.plan.ball_was_inbound);
    require(!right_ai_state.plan.ball_was_inbound);
    require(left_ai_state.plan.candidate_id >= -1);
    require(right_ai_state.plan.candidate_id >= -1);
    require_in_range(left_ai_state.plan.confidence, 0.0f, 1.0f);
    require_in_range(right_ai_state.plan.confidence, 0.0f, 1.0f);

    require_in_range(state.left.target_y, min_y, max_y);
    require_in_range(state.right.target_y, min_y, max_y);
    require_in_range(state.left.feedforward_velocity_y, -config.paddle_max_speed, config.paddle_max_speed);
    require_in_range(state.right.feedforward_velocity_y, -config.paddle_max_speed, config.paddle_max_speed);

    require(approx_equal(state.left.target_y, left_ai_state.plan.pre_contact_target_y));
}

void test_ai_replans_when_ball_direction_flips() {
    whacker::sim::Simulation simulation {};
    setup_inbound_left_state(simulation);
    const auto& config = simulation.config();

    whacker::app::MatchOptions options {};
    configure_ai_match_options(options);
    options.left_paddle_skills = {.edge = 0.12f, .power = 0.92f, .spin_inject = 0.20f};
    options.right_paddle_skills = {.edge = 0.52f, .power = 0.24f, .spin_inject = 0.64f};

    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};

    whacker::app::update_targets_for_play(
        simulation,
        options,
        left_ai_state,
        right_ai_state,
        whacker::sim::kFixedDt,
        0.0f,
        0.0f,
        nullptr);
    require(left_ai_state.plan.has_plan);
    require(left_ai_state.plan.ball_was_inbound);
    const int created_before = left_ai_state.plan.plan_created_step;

    auto& state = simulation.mutable_state();
    state.ball.velocity.x = 310.0f;
    state.ball.velocity.y = -40.0f;

    whacker::app::update_targets_for_play(
        simulation,
        options,
        left_ai_state,
        right_ai_state,
        whacker::sim::kFixedDt,
        0.0f,
        0.0f,
        nullptr);

    require(left_ai_state.plan.has_plan);
    require(!left_ai_state.plan.ball_was_inbound);
    require(left_ai_state.plan.plan_created_step > created_before);
    require(left_ai_state.plan.replan_cooldown_steps >= 0);
    require(left_ai_state.plan.replan_cooldown_steps <= 8);
    require_in_range(left_ai_state.plan.post_contact_recover_y, config.paddle_half_height, config.court_height - config.paddle_half_height);
    require(approx_equal(state.left.target_y, left_ai_state.plan.post_contact_recover_y));
}

void test_same_seeded_state_produces_deterministic_control_sequence() {
    whacker::sim::Simulation sim_a {};
    whacker::sim::Simulation sim_b {};
    setup_inbound_left_state(sim_a);
    setup_inbound_left_state(sim_b);

    whacker::app::MatchOptions options {};
    configure_ai_match_options(options);
    whacker::app::RuntimeAiState left_a {};
    whacker::app::RuntimeAiState right_a {};
    whacker::app::RuntimeAiState left_b {};
    whacker::app::RuntimeAiState right_b {};

    for (int i = 0; i < 30; ++i) {
        whacker::app::update_targets_for_play(
            sim_a,
            options,
            left_a,
            right_a,
            whacker::sim::kFixedDt,
            0.0f,
            0.0f,
            nullptr);
        whacker::app::update_targets_for_play(
            sim_b,
            options,
            left_b,
            right_b,
            whacker::sim::kFixedDt,
            0.0f,
            0.0f,
            nullptr);

        const auto& a = sim_a.state();
        const auto& b = sim_b.state();
        require(approx_equal(a.left.target_y, b.left.target_y));
        require(approx_equal(a.right.target_y, b.right.target_y));
        require(approx_equal(a.left.feedforward_velocity_y, b.left.feedforward_velocity_y));
        require(approx_equal(a.right.feedforward_velocity_y, b.right.feedforward_velocity_y));
        require(left_a.plan.has_plan == left_b.plan.has_plan);
        require(right_a.plan.has_plan == right_b.plan.has_plan);
        require(left_a.plan.candidate_id == left_b.plan.candidate_id);
        require(right_a.plan.candidate_id == right_b.plan.candidate_id);
        require(left_a.plan.state_signature == left_b.plan.state_signature);
        require(right_a.plan.state_signature == right_b.plan.state_signature);
        require(approx_equal(left_a.plan.intercept_time_s, left_b.plan.intercept_time_s, 1.0e-4f));
        require(approx_equal(right_a.plan.intercept_time_s, right_b.plan.intercept_time_s, 1.0e-4f));

        const whacker::sim::ScoreEvent score_a = sim_a.step(whacker::sim::kFixedDt);
        const whacker::sim::ScoreEvent score_b = sim_b.step(whacker::sim::kFixedDt);
        require(score_a == score_b);
        require(sim_a.state().left_score == sim_b.state().left_score);
        require(sim_a.state().right_score == sim_b.state().right_score);
        require(sim_a.state().rally_hits == sim_b.state().rally_hits);
    }
}

void test_ai_cooldown_blocks_optional_replan() {
    whacker::sim::Simulation simulation {};
    setup_inbound_left_state(simulation);

    whacker::app::MatchOptions options {};
    configure_ai_match_options(options);

    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};

    whacker::app::update_targets_for_play(
        simulation,
        options,
        left_ai_state,
        right_ai_state,
        whacker::sim::kFixedDt,
        0.0f,
        0.0f,
        nullptr);

    require(left_ai_state.plan.has_plan);
    const int created_before = left_ai_state.plan.plan_created_step;
    const std::uint64_t signature = whacker::app::compute_ai_replan_signature(simulation.state(), true);

    left_ai_state.plan.state_signature = signature;
    left_ai_state.plan.ball_was_inbound = true;
    left_ai_state.plan.valid_until_step = left_ai_state.runtime_step_counter + 120;
    left_ai_state.plan.confidence = 0.90f;
    left_ai_state.plan.intercept_time_s = 0.65f;
    left_ai_state.plan.replan_cooldown_steps = 5;

    whacker::app::update_targets_for_play(
        simulation,
        options,
        left_ai_state,
        right_ai_state,
        whacker::sim::kFixedDt,
        0.0f,
        0.0f,
        nullptr);

    require(left_ai_state.plan.has_plan);
    require(left_ai_state.plan.plan_created_step == created_before);
    require(left_ai_state.plan.replan_cooldown_steps == 4);
}

void test_ai_optional_replan_hysteresis_keeps_existing_plan() {
    whacker::sim::Simulation simulation {};
    setup_inbound_left_state(simulation);

    whacker::app::MatchOptions options {};
    configure_ai_match_options(options);

    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};

    whacker::app::update_targets_for_play(
        simulation,
        options,
        left_ai_state,
        right_ai_state,
        whacker::sim::kFixedDt,
        0.0f,
        0.0f,
        nullptr);

    require(left_ai_state.plan.has_plan);
    const int created_before = left_ai_state.plan.plan_created_step;
    const float target_before = left_ai_state.plan.pre_contact_target_y;
    const std::uint64_t signature = whacker::app::compute_ai_replan_signature(simulation.state(), true);

    left_ai_state.plan.state_signature = signature;
    left_ai_state.plan.ball_was_inbound = true;
    left_ai_state.plan.valid_until_step = left_ai_state.runtime_step_counter + 120;
    left_ai_state.plan.confidence = 0.95f;
    left_ai_state.plan.intercept_time_s = 0.70f;
    left_ai_state.plan.replan_cooldown_steps = 0;
    left_ai_state.plan.decision_score = 1000.0f;

    whacker::app::update_targets_for_play(
        simulation,
        options,
        left_ai_state,
        right_ai_state,
        whacker::sim::kFixedDt,
        0.0f,
        0.0f,
        nullptr);

    require(left_ai_state.plan.has_plan);
    require(left_ai_state.plan.plan_created_step == created_before);
    require(left_ai_state.plan.replan_cooldown_steps == 1);
    require(approx_equal(left_ai_state.plan.pre_contact_target_y, target_before));
}

void test_ai_forced_replan_overrides_cooldown_on_low_confidence_near_contact() {
    whacker::sim::Simulation simulation {};
    setup_inbound_left_state(simulation);

    whacker::app::MatchOptions options {};
    configure_ai_match_options(options);

    whacker::app::RuntimeAiState left_ai_state {};
    whacker::app::RuntimeAiState right_ai_state {};

    whacker::app::update_targets_for_play(
        simulation,
        options,
        left_ai_state,
        right_ai_state,
        whacker::sim::kFixedDt,
        0.0f,
        0.0f,
        nullptr);

    require(left_ai_state.plan.has_plan);
    const int created_before = left_ai_state.plan.plan_created_step;
    const std::uint64_t signature = whacker::app::compute_ai_replan_signature(simulation.state(), true);

    left_ai_state.plan.state_signature = signature;
    left_ai_state.plan.ball_was_inbound = true;
    left_ai_state.plan.valid_until_step = left_ai_state.runtime_step_counter + 120;
    left_ai_state.plan.confidence = 0.10f;
    left_ai_state.plan.intercept_time_s = 0.05f;
    left_ai_state.plan.replan_cooldown_steps = 8;
    left_ai_state.plan.decision_score = 1000.0f;

    whacker::app::update_targets_for_play(
        simulation,
        options,
        left_ai_state,
        right_ai_state,
        whacker::sim::kFixedDt,
        0.0f,
        0.0f,
        nullptr);

    require(left_ai_state.plan.has_plan);
    require(left_ai_state.plan.plan_created_step > created_before);
    require(left_ai_state.plan.replan_cooldown_steps >= 2);
    require(left_ai_state.plan.replan_cooldown_steps <= 8);
}

}  // namespace

int main() {
    test_ai_mode_produces_bounded_targets_and_plan_state();
    test_ai_replans_when_ball_direction_flips();
    test_same_seeded_state_produces_deterministic_control_sequence();
    test_ai_cooldown_blocks_optional_replan();
    test_ai_optional_replan_hysteresis_keeps_existing_plan();
    test_ai_forced_replan_overrides_cooldown_on_low_confidence_near_contact();
    return 0;
}
