#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "ai_core.hpp"
#include "progression/skills.hpp"

namespace {

struct StyleMetrics {
    int samples = 0;
    float sum_impact = 0.0f;
    float sum_abs_spin_delta = 0.0f;
    float sum_clean = 0.0f;
    int stabilize_count = 0;
    int pressure_count = 0;
    int spintrap_count = 0;
};

struct DecisionTrace {
    whacker::app::AiIntent intent = whacker::app::AiIntent::Stabilize;
    int candidate_id = -1;
    float contact_u = 0.0f;
    float strike_vy = 0.0f;
    float target_y = 0.0f;
};

bool approx_equal(const float a, const float b, const float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

void require_impl(const bool condition, const char* expression, const int line) {
    if (!condition) {
        std::fprintf(stderr, "ai_style_fidelity_smoke assertion failed at line %d: %s\n", line, expression);
        std::abort();
    }
}

#define REQUIRE(condition) require_impl((condition), #condition, __LINE__)

whacker::sim::Simulation make_probe_simulation(const int sample_index) {
    whacker::sim::Simulation simulation {};
    auto& state = simulation.mutable_state();
    const auto& config = simulation.config();

    const float y_seed = static_cast<float>((sample_index * 37) % 250);
    const float vy_seed = static_cast<float>((sample_index * 19) % 220);
    const float spin_seed = static_cast<float>((sample_index * 23) % 140);

    state.ball.position.x = 250.0f + static_cast<float>((sample_index % 5) * 12);
    state.ball.position.y = config.ball_radius + 18.0f + y_seed;
    state.ball.velocity.x = -240.0f - static_cast<float>((sample_index % 7) * 14);
    state.ball.velocity.y = -110.0f + vy_seed;
    state.ball.spin = -3.5f + (spin_seed * 0.05f);
    const float speed_step = static_cast<float>(sample_index % 6);
    state.ball.speed_scalar = 1.0f + (speed_step * 0.07f);
    state.left.center_y = 72.0f + static_cast<float>((sample_index * 11) % 220);
    state.left.velocity_y = -80.0f + static_cast<float>((sample_index * 13) % 160);
    state.right.center_y = 84.0f + static_cast<float>((sample_index * 17) % 210);
    state.right.velocity_y = -70.0f + static_cast<float>((sample_index * 7) % 140);
    return simulation;
}

StyleMetrics gather_custom_skill_metrics(
    const whacker::progression::SkillState& skills,
    const int sample_count,
    std::vector<DecisionTrace>* trace_out = nullptr) {
    StyleMetrics metrics {};
    whacker::progression::SkillState clamped = skills;
    whacker::progression::clamp_skills(clamped);

    whacker::app::RuntimeAiState ai_state {};
    ai_state.initialized = true;
    ai_state.style = whacker::app::AiStyle::Balanced;
    ai_state.skills = clamped;

    if (trace_out != nullptr) {
        trace_out->clear();
        trace_out->reserve(static_cast<std::size_t>(sample_count));
    }

    for (int i = 0; i < sample_count; ++i) {
        whacker::sim::Simulation simulation = make_probe_simulation(i);
        const whacker::app::AiDecision decision = whacker::app::plan_ai_decision(
            simulation,
            true,
            ai_state,
            static_cast<std::uint64_t>(1000 + i),
            false);

        if (!decision.valid || !decision.inbound) {
            continue;
        }

        ++metrics.samples;
        metrics.sum_impact += decision.expected_impact_factor;
        metrics.sum_abs_spin_delta += std::fabs(decision.expected_spin_delta);
        metrics.sum_clean += decision.clean_contact_metric;

        if (decision.intent == whacker::app::AiIntent::Stabilize) {
            ++metrics.stabilize_count;
        } else if (decision.intent == whacker::app::AiIntent::Pressure) {
            ++metrics.pressure_count;
        } else {
            ++metrics.spintrap_count;
        }

        if (trace_out != nullptr) {
            trace_out->push_back(DecisionTrace {
                .intent = decision.intent,
                .candidate_id = decision.candidate_id,
                .contact_u = decision.contact_u,
                .strike_vy = decision.strike_feedforward_vy,
                .target_y = decision.pre_contact_target_y,
            });
        }
    }

    return metrics;
}

float avg_or_zero(const float sum, const int count) {
    if (count <= 0) {
        return 0.0f;
    }
    return sum / static_cast<float>(count);
}

void test_style_directional_separation() {
    constexpr int kSamples = 400;

    const StyleMetrics power = gather_custom_skill_metrics(
        whacker::progression::SkillState {.edge = 0.06f, .power = 0.40f, .spin_inject = 0.04f},
        kSamples);
    const StyleMetrics technical = gather_custom_skill_metrics(
        whacker::progression::SkillState {.edge = 0.40f, .power = 0.06f, .spin_inject = 0.04f},
        kSamples);
    const StyleMetrics spin = gather_custom_skill_metrics(
        whacker::progression::SkillState {.edge = 0.04f, .power = 0.06f, .spin_inject = 0.40f},
        kSamples);

    REQUIRE(power.samples > 220);
    REQUIRE(technical.samples > 220);
    REQUIRE(spin.samples > 220);

    const float power_impact = avg_or_zero(power.sum_impact, power.samples);
    const float technical_impact = avg_or_zero(technical.sum_impact, technical.samples);
    const float power_spin = avg_or_zero(power.sum_abs_spin_delta, power.samples);
    const float spin_spin = avg_or_zero(spin.sum_abs_spin_delta, spin.samples);
    const float technical_clean = avg_or_zero(technical.sum_clean, technical.samples);
    const float power_clean = avg_or_zero(power.sum_clean, power.samples);

    REQUIRE(power_impact >= (technical_impact + 0.03f));
    REQUIRE(spin_spin >= (power_spin * 1.05f));
    REQUIRE(technical_clean >= (power_clean - 0.10f));
}

void test_balanced_metrics_stay_between_extremes() {
    constexpr int kSamples = 400;
    const StyleMetrics power = gather_custom_skill_metrics(
        whacker::progression::SkillState {.edge = 0.06f, .power = 0.40f, .spin_inject = 0.04f},
        kSamples);
    const StyleMetrics technical = gather_custom_skill_metrics(
        whacker::progression::SkillState {.edge = 0.40f, .power = 0.06f, .spin_inject = 0.04f},
        kSamples);
    const StyleMetrics spin = gather_custom_skill_metrics(
        whacker::progression::SkillState {.edge = 0.04f, .power = 0.06f, .spin_inject = 0.40f},
        kSamples);
    const StyleMetrics balanced = gather_custom_skill_metrics(
        whacker::progression::SkillState {.edge = 0.34f, .power = 0.33f, .spin_inject = 0.33f},
        kSamples);

    REQUIRE(balanced.samples > 220);

    const float impact_power = avg_or_zero(power.sum_impact, power.samples);
    const float impact_technical = avg_or_zero(technical.sum_impact, technical.samples);
    const float impact_spin = avg_or_zero(spin.sum_impact, spin.samples);
    const float impact_balanced = avg_or_zero(balanced.sum_impact, balanced.samples);

    const float spin_power = avg_or_zero(power.sum_abs_spin_delta, power.samples);
    const float spin_spin = avg_or_zero(spin.sum_abs_spin_delta, spin.samples);
    const float spin_balanced = avg_or_zero(balanced.sum_abs_spin_delta, balanced.samples);

    const float impact_min = std::min(impact_power, std::min(impact_technical, impact_spin));
    const float impact_max = std::max(impact_power, std::max(impact_technical, impact_spin));

    REQUIRE(impact_balanced >= (impact_min - 0.05f));
    REQUIRE(impact_balanced <= (impact_max + 0.05f));
    REQUIRE(spin_balanced >= (spin_power * 0.90f));
    REQUIRE(spin_balanced <= (spin_spin * 1.05f));
}

void test_seeded_decisions_are_reproducible() {
    std::vector<DecisionTrace> trace_a;
    std::vector<DecisionTrace> trace_b;

    (void)gather_custom_skill_metrics(
        whacker::progression::SkillState {.edge = 0.02f, .power = 0.04f, .spin_inject = 0.40f},
        240,
        &trace_a);
    (void)gather_custom_skill_metrics(
        whacker::progression::SkillState {.edge = 0.02f, .power = 0.04f, .spin_inject = 0.40f},
        240,
        &trace_b);

    REQUIRE(trace_a.size() == trace_b.size());
    for (std::size_t i = 0; i < trace_a.size(); ++i) {
        REQUIRE(trace_a[i].intent == trace_b[i].intent);
        REQUIRE(trace_a[i].candidate_id == trace_b[i].candidate_id);
        REQUIRE(approx_equal(trace_a[i].contact_u, trace_b[i].contact_u));
        REQUIRE(approx_equal(trace_a[i].strike_vy, trace_b[i].strike_vy));
        REQUIRE(approx_equal(trace_a[i].target_y, trace_b[i].target_y));
    }
}

}  // namespace

int main() {
    test_style_directional_separation();
    test_balanced_metrics_stay_between_extremes();
    test_seeded_decisions_are_reproducible();
    return 0;
}
