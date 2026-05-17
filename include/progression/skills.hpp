#pragma once

#include <cstdint>

#include "sim/config.hpp"

namespace whacker::progression {

constexpr float kSkillBudgetCap = 1.70f;

struct SkillState {
    float edge = 0.0f;
    float power = 0.0f;
    float spin_inject = 0.0f;
};

struct SkillUsageMetrics {
    float edge = 0.0f;
    float power = 0.0f;
    float spin_inject = 0.0f;
    float exposure = 0.0f;
};

struct SkillGrowthConfig {
    float k_edge_growth = 0.00120f;
    float k_power_growth = 0.00110f;
    float k_spin_inject_growth = 0.00125f;
};

struct SkillUsageAccumulator {
    int contacts = 0;
    int clean_contacts = 0;
    int high_edge_contacts = 0;

    float sum_abs_u = 0.0f;
    float sum_power_samples = 0.0f;
    float sum_spin_inject_samples = 0.0f;
};

void clamp_skills(SkillState& skills);
void clamp_usage(SkillUsageMetrics& usage);

void accumulate_contact_usage(
    SkillUsageAccumulator& acc,
    float contact_u,
    float paddle_velocity_at_impact,
    float ball_speed_at_impact,
    const sim::SimulationConfig& base_config);

SkillUsageMetrics finalize_usage(const SkillUsageAccumulator& acc);
float clean_contact_rate(const SkillUsageAccumulator& acc);
float high_edge_contact_rate(const SkillUsageAccumulator& acc);

void apply_skill_growth(
    SkillState& skills,
    const SkillUsageMetrics& usage,
    const SkillGrowthConfig& growth = {});

}  // namespace whacker::progression
