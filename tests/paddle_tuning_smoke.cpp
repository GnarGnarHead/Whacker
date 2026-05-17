#include <cstdlib>
#include <cmath>

#include "paddle_tuning.hpp"

namespace {

bool approx_equal(const float a, const float b, const float eps = 1.0e-4f) {
    return std::fabs(a - b) <= eps;
}

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

void test_tuning_from_skills_clamps_to_skill_and_budget_limits() {
    const whacker::progression::SkillState skills {
        .edge = 1.40f,
        .power = 1.00f,
        .spin_inject = 1.00f
    };
    const whacker::app::PaddleTuning tuning = whacker::app::paddle_tuning_from_skills(skills);
    require(tuning.edge >= 0.0f && tuning.edge <= 1.0f);
    require(tuning.power >= 0.0f && tuning.power <= 1.0f);
    require(tuning.spin_inject >= 0.0f && tuning.spin_inject <= 1.0f);
    require(approx_equal(tuning.edge + tuning.power + tuning.spin_inject, 1.70f, 1.0e-3f));
    require(approx_equal(tuning.budget, 1.70f, 1.0e-3f));
}

void test_tuning_to_skills_is_direct_for_valid_bar_values() {
    whacker::app::PaddleTuning tuning {};
    tuning.edge = 0.70f;
    tuning.power = 0.20f;
    tuning.spin_inject = 0.10f;
    tuning.budget = 0.00f;
    const whacker::progression::SkillState skills = whacker::app::paddle_tuning_to_skills(tuning);
    require(approx_equal(skills.edge, 0.70f));
    require(approx_equal(skills.power, 0.20f));
    require(approx_equal(skills.spin_inject, 0.10f));
}

void test_paddle_tuning_style_classification() {
    whacker::app::PaddleTuning balanced {};
    balanced.edge = 0.34f;
    balanced.power = 0.33f;
    balanced.spin_inject = 0.33f;
    require(whacker::app::paddle_tuning_style(balanced) == whacker::app::AiStyle::Balanced);

    whacker::app::PaddleTuning power {};
    power.edge = 0.10f;
    power.power = 0.80f;
    power.spin_inject = 0.10f;
    require(whacker::app::paddle_tuning_style(power) == whacker::app::AiStyle::Power);

    whacker::app::PaddleTuning technical {};
    technical.edge = 0.80f;
    technical.power = 0.10f;
    technical.spin_inject = 0.10f;
    require(whacker::app::paddle_tuning_style(technical) == whacker::app::AiStyle::Technical);

    whacker::app::PaddleTuning spin {};
    spin.edge = 0.10f;
    spin.power = 0.10f;
    spin.spin_inject = 0.80f;
    require(whacker::app::paddle_tuning_style(spin) == whacker::app::AiStyle::Spin);
}

void test_nudge_paddle_tuning_stays_on_simplex() {
    whacker::app::PaddleTuning tuning {};
    tuning.edge = 0.40f;
    tuning.power = 0.40f;
    tuning.spin_inject = 0.40f;
    tuning.budget = 0.0f;
    whacker::app::normalize_paddle_tuning(tuning);
    const float sum_before = tuning.edge + tuning.power + tuning.spin_inject;
    whacker::app::nudge_paddle_tuning(tuning, 4.0f, 4.0f);
    const float sum_after = tuning.edge + tuning.power + tuning.spin_inject;
    require(approx_equal(sum_after, sum_before, 1.0e-3f));
    require(tuning.edge >= 0.0f);
    require(tuning.power >= 0.0f);
    require(tuning.spin_inject >= 0.0f);
}

}  // namespace

int main() {
    test_tuning_from_skills_clamps_to_skill_and_budget_limits();
    test_tuning_to_skills_is_direct_for_valid_bar_values();
    test_paddle_tuning_style_classification();
    test_nudge_paddle_tuning_stays_on_simplex();
    return 0;
}
