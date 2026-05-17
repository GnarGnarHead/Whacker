#pragma once

#include <cstdint>

#include "app_types.hpp"
#include "sim/physics.hpp"

namespace whacker::app {

enum class AiIntent : std::uint8_t {
    Stabilize = 0,
    Pressure = 1,
    SpinTrap = 2,
};

struct AiPerception {
    bool inbound = false;
    bool predicted = false;
    float contact_plane_x = 0.0f;
    float intercept_time_s = 0.0f;
    float intercept_y = 0.0f;
    float intercept_vy = 0.0f;
    int wall_bounces = 0;
    float confidence = 0.0f;
};

struct AiCandidate {
    int id = -1;
    AiIntent intent = AiIntent::Stabilize;
    float contact_u = 0.0f;
    float strike_vy = 0.0f;
    float required_center_y = 0.0f;
    float required_speed_ratio = 0.0f;
    float reach_slack = 0.0f;
    float make_term = 0.0f;
    float quality_term = 0.0f;
    float style_term = 0.0f;
    float risk_term = 0.0f;
    float motion_term = 0.0f;
    float score = 0.0f;
    float impact_factor = 0.0f;
    float spin_delta_estimate = 0.0f;
    float clean_contact_metric = 0.0f;
};

struct AiDecision {
    bool valid = false;
    bool inbound = false;
    AiIntent intent = AiIntent::Stabilize;
    int candidate_id = -1;

    float intercept_time_s = 0.0f;
    float intercept_y = 0.0f;
    float contact_u = 0.0f;
    // World-frame Y command (+down, -up). This is not mirrored across sides.
    float strike_feedforward_vy = 0.0f;
    float pre_contact_target_y = 0.0f;
    float post_contact_recover_y = 0.0f;

    float confidence = 0.0f;
    float score = 0.0f;
    int valid_steps = 6;
    int cooldown_steps = 2;

    int coarse_candidate_count = 0;
    int scored_candidate_count = 0;
    int reachable_candidate_count = 0;
    int predicted_wall_bounces = 0;

    float make_term = 0.0f;
    float quality_term = 0.0f;
    float style_term = 0.0f;
    float risk_term = 0.0f;
    float motion_term = 0.0f;
    float make_contact_probability = 0.0f;
    float reach_slack = 0.0f;
    int miss_risk_level = 0;

    float expected_impact_factor = 0.0f;
    // Actor-frame diagnostic term used by planner scoring/telemetry.
    float expected_spin_delta = 0.0f;
    float clean_contact_metric = 0.0f;

    float style_mix_power = 0.0f;
    float style_mix_technical = 0.0f;
    float style_mix_spin = 0.0f;
    float intent_weight_stabilize = 0.0f;
    float intent_weight_pressure = 0.0f;
    float intent_weight_spintrap = 0.0f;

    float strike_commit_window_s = 0.0f;
    float strike_min_make_prob = 0.0f;
    float strike_velocity_target_abs = 0.0f;
};

struct AiPlannerConfig {
    int predictor_max_steps_inbound = 720;
    int reachability_max_steps = 360;
    int max_candidates = 24;
};

AiDecision plan_ai_decision(
    const whacker::sim::Simulation& simulation,
    bool for_left_paddle,
    const RuntimeAiState& ai_state,
    std::uint64_t decision_counter,
    bool ambient_mode,
    const AiPlannerConfig& config = {});

void apply_ai_decision(
    whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config,
    const AiDecision& decision,
    bool inbound,
    float intercept_time_s,
    bool ambient_mode);

uint64_t compute_ai_replan_signature(const whacker::sim::RallyState& state, bool for_left_paddle);
uint64_t compute_ai_state_signature(const whacker::sim::Simulation& simulation, bool for_left_paddle);

}  // namespace whacker::app
