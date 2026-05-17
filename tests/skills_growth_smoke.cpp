#include <cmath>
#include <cstdlib>

#include "progression/skills.hpp"

namespace {

bool approx_equal(const float a, const float b, const float eps = 1.0e-6f) {
    return std::fabs(a - b) <= eps;
}

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

void test_apply_skill_growth_matches_linear_usage_formula() {
    whacker::progression::SkillState skills {
        .edge = 0.10f,
        .power = 0.10f,
        .spin_inject = 0.10f
    };
    const whacker::progression::SkillUsageMetrics usage {
        .edge = 0.0f,
        .power = 1.0f,
        .spin_inject = 0.0f,
        .exposure = 12.0f
    };

    whacker::progression::apply_skill_growth(skills, usage);

    // delta = k_power_growth * usage * exposure * (1 - current)
    const float expected_power = 0.10f + (0.00110f * 1.0f * 12.0f * 0.90f);
    require(approx_equal(skills.edge, 0.10f));
    require(approx_equal(skills.power, expected_power));
    require(approx_equal(skills.spin_inject, 0.10f));
}

void test_higher_usage_produces_proportionally_higher_growth() {
    whacker::progression::SkillState low_usage_skills {
        .edge = 0.10f,
        .power = 0.20f,
        .spin_inject = 0.10f
    };
    whacker::progression::SkillState high_usage_skills = low_usage_skills;

    const whacker::progression::SkillUsageMetrics low_usage {
        .edge = 0.0f,
        .power = 0.30f,
        .spin_inject = 0.0f,
        .exposure = 20.0f
    };
    const whacker::progression::SkillUsageMetrics high_usage {
        .edge = 0.0f,
        .power = 0.80f,
        .spin_inject = 0.0f,
        .exposure = 20.0f
    };

    whacker::progression::apply_skill_growth(low_usage_skills, low_usage);
    whacker::progression::apply_skill_growth(high_usage_skills, high_usage);

    const float low_delta = low_usage_skills.power - 0.20f;
    const float high_delta = high_usage_skills.power - 0.20f;
    require(high_delta > low_delta);
    require(approx_equal(high_delta / low_delta, 0.80f / 0.30f, 1.0e-4f));
}

void test_higher_exposure_produces_proportionally_higher_growth() {
    whacker::progression::SkillState low_exposure_skills {
        .edge = 0.10f,
        .power = 0.20f,
        .spin_inject = 0.10f
    };
    whacker::progression::SkillState high_exposure_skills = low_exposure_skills;

    const whacker::progression::SkillUsageMetrics low_exposure {
        .edge = 0.0f,
        .power = 0.60f,
        .spin_inject = 0.0f,
        .exposure = 15.0f
    };
    const whacker::progression::SkillUsageMetrics high_exposure {
        .edge = 0.0f,
        .power = 0.60f,
        .spin_inject = 0.0f,
        .exposure = 45.0f
    };

    whacker::progression::apply_skill_growth(low_exposure_skills, low_exposure);
    whacker::progression::apply_skill_growth(high_exposure_skills, high_exposure);

    const float low_delta = low_exposure_skills.power - 0.20f;
    const float high_delta = high_exposure_skills.power - 0.20f;
    require(high_delta > low_delta);
    require(approx_equal(high_delta / low_delta, 45.0f / 15.0f, 1.0e-4f));
}

void test_power_usage_rewards_dead_center_more_than_off_center() {
    const whacker::sim::SimulationConfig config {};

    whacker::progression::SkillUsageAccumulator center_acc {};
    whacker::progression::accumulate_contact_usage(
        center_acc,
        0.0f,
        0.0f,
        config.ball_base_speed,
        config);
    const whacker::progression::SkillUsageMetrics center_usage =
        whacker::progression::finalize_usage(center_acc);

    whacker::progression::SkillUsageAccumulator off_center_acc {};
    whacker::progression::accumulate_contact_usage(
        off_center_acc,
        0.25f,
        0.0f,
        config.ball_base_speed,
        config);
    const whacker::progression::SkillUsageMetrics off_center_usage =
        whacker::progression::finalize_usage(off_center_acc);

    require(approx_equal(center_usage.exposure, 1.0f));
    require(approx_equal(off_center_usage.exposure, 1.0f));
    require(center_usage.power > off_center_usage.power);
}

}  // namespace

int main() {
    test_apply_skill_growth_matches_linear_usage_formula();
    test_higher_usage_produces_proportionally_higher_growth();
    test_higher_exposure_produces_proportionally_higher_growth();
    test_power_usage_rewards_dead_center_more_than_off_center();
    return 0;
}
