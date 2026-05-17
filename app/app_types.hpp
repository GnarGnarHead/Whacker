#pragma once

#include <cstdint>

#include "ai_style_catalog.hpp"
#include "progression/skills.hpp"
#include "progression/style.hpp"

namespace whacker::app {

enum class PaddleMode : std::uint8_t {
    Human = 0,
    AI = 1
};

struct RuntimeAiPlanState {
    bool has_plan = false;
    int plan_created_step = 0;
    int valid_until_step = 0;
    int replan_cooldown_steps = 0;
    float intercept_time_s = 0.0f;
    float intercept_y = 0.0f;
    float contact_u = 0.0f;
    float strike_feedforward_vy = 0.0f;
    float pre_contact_target_y = 0.0f;
    float post_contact_recover_y = 0.0f;
    float confidence = 0.0f;
    float decision_score = 0.0f;
    std::uint64_t state_signature = 0ULL;
    bool ball_was_inbound = false;
    std::uint8_t intent_id = 0U;

    int candidate_id = -1;
    int coarse_candidate_count = 0;
    int scored_candidate_count = 0;
    int reachable_candidate_count = 0;
    int predicted_wall_bounces = 0;

    float expected_impact_factor = 0.0f;
    float expected_spin_delta = 0.0f;
    float clean_contact_metric = 0.0f;
    float make_contact_probability = 0.0f;
    float reach_slack = 0.0f;
    int miss_risk_level = 0;

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

struct MatchOptions {
    PaddleMode left_mode = PaddleMode::Human;
    PaddleMode right_mode = PaddleMode::AI;
    AiStyle left_ai_style = AiStyle::Balanced;
    AiStyle right_ai_style = AiStyle::Power;
    whacker::progression::SkillState left_paddle_skills = ai_style_profile(AiStyle::Balanced).seed_skills;
    whacker::progression::SkillState right_paddle_skills = ai_style_profile(AiStyle::Power).seed_skills;
};

inline bool options_equal(const MatchOptions& a, const MatchOptions& b) {
    return
        (a.left_mode == b.left_mode) &&
        (a.right_mode == b.right_mode) &&
        (a.left_ai_style == b.left_ai_style) &&
        (a.right_ai_style == b.right_ai_style) &&
        (a.left_paddle_skills.edge == b.left_paddle_skills.edge) &&
        (a.left_paddle_skills.power == b.left_paddle_skills.power) &&
        (a.left_paddle_skills.spin_inject == b.left_paddle_skills.spin_inject) &&
        (a.right_paddle_skills.edge == b.right_paddle_skills.edge) &&
        (a.right_paddle_skills.power == b.right_paddle_skills.power) &&
        (a.right_paddle_skills.spin_inject == b.right_paddle_skills.spin_inject);
}

struct RuntimeAiState {
    bool initialized = false;
    AiStyle style = AiStyle::Balanced;
    whacker::progression::SkillState skills {};
    whacker::progression::RivalStyleProfile profile {};
    std::uint64_t decision_counter = 0ULL;
    int runtime_step_counter = 0;
    RuntimeAiPlanState plan {};
};

struct PlayControlOverrides {
    bool force_modes = false;
    PaddleMode left_mode = PaddleMode::Human;
    PaddleMode right_mode = PaddleMode::AI;
    bool ai_training_context = false;
    bool override_left_skills = false;
    bool override_right_skills = false;
    whacker::progression::SkillState left_skills {};
    whacker::progression::SkillState right_skills {};
};

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
};

}  // namespace whacker::app
