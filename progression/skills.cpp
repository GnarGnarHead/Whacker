#include "progression/skills.hpp"

#include <algorithm>
#include <cmath>

namespace whacker::progression {

namespace {

float clamp01(const float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float growth_step(
    const float skill,
    const float usage,
    const float exposure,
    const float growth_rate) {
    const float s = clamp01(skill);
    const float u = clamp01(usage);
    const float e = std::max(0.0f, exposure);
    return clamp01(s + (growth_rate * u * e * (1.0f - s)));
}

}  // namespace

void clamp_skills(SkillState& skills) {
    skills.edge = clamp01(skills.edge);
    skills.power = clamp01(skills.power);
    skills.spin_inject = clamp01(skills.spin_inject);

    const float sum = skills.edge + skills.power + skills.spin_inject;
    if (sum > kSkillBudgetCap && sum > 1.0e-6f) {
        const float scale = kSkillBudgetCap / sum;
        skills.edge *= scale;
        skills.power *= scale;
        skills.spin_inject *= scale;
    }
}

void clamp_usage(SkillUsageMetrics& usage) {
    usage.edge = clamp01(usage.edge);
    usage.power = clamp01(usage.power);
    usage.spin_inject = clamp01(usage.spin_inject);
    usage.exposure = std::max(0.0f, usage.exposure);
}

void accumulate_contact_usage(
    SkillUsageAccumulator& acc,
    const float contact_u,
    const float paddle_velocity_at_impact,
    const float ball_speed_at_impact,
    const sim::SimulationConfig& base_config) {
    const float abs_u = clamp01(std::abs(contact_u));
    const float max_paddle_speed = std::max(base_config.paddle_max_speed, 1.0e-6f);
    const float speed_norm_window = std::max(1.5f * base_config.ball_base_speed, 1.0e-6f);
    const float speed_norm = clamp01((ball_speed_at_impact - base_config.ball_base_speed) / speed_norm_window);

    const float center = 1.0f - abs_u;
    const float power_sample = center * (0.55f + (0.45f * speed_norm));
    const float spin_inject_sample = clamp01(std::abs(paddle_velocity_at_impact) / max_paddle_speed);
    ++acc.contacts;
    acc.sum_abs_u += abs_u;
    acc.sum_power_samples += power_sample;
    acc.sum_spin_inject_samples += spin_inject_sample;

    if (abs_u >= 0.75f) {
        ++acc.high_edge_contacts;
    }

    const float u_clean_threshold = 0.30f;
    const float pv_clean_threshold = 0.35f * base_config.paddle_max_speed;
    const bool clean_contact =
        (abs_u <= u_clean_threshold) && (std::abs(paddle_velocity_at_impact) <= pv_clean_threshold);
    if (!clean_contact) {
        return;
    }

    ++acc.clean_contacts;
}

SkillUsageMetrics finalize_usage(const SkillUsageAccumulator& acc) {
    SkillUsageMetrics usage {};
    if (acc.contacts > 0) {
        const float contacts = static_cast<float>(acc.contacts);
        usage.edge = clamp01(acc.sum_abs_u / contacts);
        usage.power = clamp01(acc.sum_power_samples / contacts);
        usage.spin_inject = clamp01(acc.sum_spin_inject_samples / contacts);
        usage.exposure = contacts;
    }

    clamp_usage(usage);
    return usage;
}

float clean_contact_rate(const SkillUsageAccumulator& acc) {
    if (acc.contacts <= 0) {
        return 0.0f;
    }
    return clamp01(static_cast<float>(acc.clean_contacts) / static_cast<float>(acc.contacts));
}

float high_edge_contact_rate(const SkillUsageAccumulator& acc) {
    if (acc.contacts <= 0) {
        return 0.0f;
    }
    return clamp01(static_cast<float>(acc.high_edge_contacts) / static_cast<float>(acc.contacts));
}

void apply_skill_growth(
    SkillState& skills,
    const SkillUsageMetrics& usage,
    const SkillGrowthConfig& growth) {
    skills.edge = growth_step(skills.edge, usage.edge, usage.exposure, growth.k_edge_growth);
    skills.power = growth_step(skills.power, usage.power, usage.exposure, growth.k_power_growth);
    skills.spin_inject = growth_step(skills.spin_inject, usage.spin_inject, usage.exposure, growth.k_spin_inject_growth);

    clamp_skills(skills);
}

}  // namespace whacker::progression
