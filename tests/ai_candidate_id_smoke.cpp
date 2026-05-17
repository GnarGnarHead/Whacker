#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "ai_candidates.hpp"
#include "ai_predict.hpp"
#include "ai_profile.hpp"
#include "ai_reachability.hpp"

namespace {

namespace app = whacker::app;
namespace sim = whacker::sim;

void require_impl(const bool condition, const char* expression, const int line) {
    if (!condition) {
        std::fprintf(stderr, "ai_candidate_id_smoke assertion failed at line %d: %s\n", line, expression);
        std::abort();
    }
}

#define REQUIRE(condition) require_impl((condition), #condition, __LINE__)

bool approx_equal(const float a, const float b, const float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

app::RuntimeAiState make_ai_state() {
    app::RuntimeAiState ai {};
    ai.initialized = true;
    ai.style = app::AiStyle::Spin;
    ai.skills = whacker::progression::SkillState {
        .edge = 0.22f,
        .power = 0.18f,
        .spin_inject = 0.60f};
    return ai;
}

void seed_state(sim::Simulation& simulation) {
    auto& state = simulation.mutable_state();
    state.ball.position.x = 480.0f;
    state.ball.position.y = 138.0f;
    state.ball.velocity.x = -305.0f;
    state.ball.velocity.y = 68.0f;
    state.ball.spin = 2.1f;
    state.ball.speed_scalar = 1.06f;
    state.left.center_y = 252.0f;
    state.left.velocity_y = -42.0f;
    state.right.center_y = 120.0f;
    state.right.velocity_y = 18.0f;
    state.left_score = 2;
    state.right_score = 3;
    state.rally_hits = 9;
}

void test_candidate_ids_are_unique_and_stable() {
    sim::Simulation simulation {};
    seed_state(simulation);

    const app::RuntimeAiState ai_state = make_ai_state();
    const float competence = app::ai_internal::competence_from_skills(ai_state.skills);
    const app::ai_internal::AiCapabilityProfile capability =
        app::ai_internal::capability_profile_for(competence);
    const app::ai_internal::StyleMix mix = app::ai_internal::style_mix_from_skills(ai_state);
    const app::ai_internal::IntentWeights weights = app::ai_internal::intent_weights_from_mix(mix);

    sim::RallyState perceived = simulation.state();
    if (capability.reaction_lag_s > 0.0f) {
        perceived.ball.position.x -= perceived.ball.velocity.x * capability.reaction_lag_s;
        perceived.ball.position.y = app::ai_internal::clampf(
            perceived.ball.position.y - (perceived.ball.velocity.y * capability.reaction_lag_s),
            simulation.config().ball_radius,
            simulation.config().court_height - simulation.config().ball_radius);
    }

    const app::ai_internal::PredictorResult prediction =
        app::ai_internal::predict_intercept(perceived, simulation.config(), 720);
    REQUIRE(prediction.predicted);

    const float planned_intercept_y = app::ai_internal::clampf(
        prediction.intercept_y,
        simulation.config().paddle_half_height,
        simulation.config().court_height - simulation.config().paddle_half_height);

    const app::ai_internal::ReachabilityEnvelope envelope =
        app::ai_internal::compute_reachability_envelope(
            simulation.state().left,
            simulation.config(),
            prediction.t_hit,
            360,
            capability.speed_scale,
            capability.accel_scale);

    const app::ai_internal::CandidateGenerationResult first =
        app::ai_internal::generate_scored_candidates(
            simulation.state(),
            simulation.config(),
            simulation.state().left,
            simulation.state().right,
            ai_state,
            capability,
            mix,
            weights,
            prediction,
            envelope,
            planned_intercept_y);
    const app::ai_internal::CandidateGenerationResult second =
        app::ai_internal::generate_scored_candidates(
            simulation.state(),
            simulation.config(),
            simulation.state().left,
            simulation.state().right,
            ai_state,
            capability,
            mix,
            weights,
            prediction,
            envelope,
            planned_intercept_y);

    REQUIRE(first.candidate_count > 0);
    REQUIRE(first.candidate_count == second.candidate_count);

    int last_id = -1;
    for (int i = 0; i < first.candidate_count; ++i) {
        const auto& a = first.candidates[static_cast<std::size_t>(i)];
        const auto& b = second.candidates[static_cast<std::size_t>(i)];

        REQUIRE(a.id > last_id);
        last_id = a.id;

        REQUIRE(a.id == b.id);
        REQUIRE(a.intent == b.intent);
        REQUIRE(approx_equal(a.contact_u, b.contact_u));
        REQUIRE(approx_equal(a.strike_vy, b.strike_vy));
        REQUIRE(approx_equal(a.score, b.score));
    }
}

}  // namespace

int main() {
    test_candidate_ids_are_unique_and_stable();
    return 0;
}
