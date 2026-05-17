#include "play_control.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>
#include <cmath>

#include <GLFW/glfw3.h>

#include "ai_core.hpp"
#include "ai_plan_state_bridge.hpp"
#include "ai_style_catalog.hpp"
#include "paddle_tuning.hpp"
#include "play_control_human.hpp"
#include "sim/math.hpp"

namespace {

constexpr float kInboundEpsilon = 1.0e-4f;
constexpr float kPlanHysteresisThreshold = 0.06f;
constexpr float kMinConfidenceReplan = 0.35f;
constexpr float kLowConfidenceNearContactWindowS = 0.30f;
constexpr whacker::app::AiPlannerConfig kPlayPlannerConfig {};
constexpr whacker::app::AiPlannerConfig kAmbientPlannerConfig {
    .predictor_max_steps_inbound = 360,
    .reachability_max_steps = 240,
    .max_candidates = 12,
};

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

struct AiReplanPolicy {
    bool forced_replan = false;
    bool optional_replan = false;
    bool should_replan = false;
};

float skill_scale_with_floor(const float skill_value, const float floor_scale) {
    const float floor = clampf(floor_scale, 0.0f, 1.0f);
    const float skill = clampf(skill_value, 0.0f, 1.0f);
    return floor + ((1.0f - floor) * skill);
}

bool ball_moving_toward_paddle(const whacker::sim::RallyState& state, const bool for_left_paddle) {
    return for_left_paddle ? (state.ball.velocity.x < -kInboundEpsilon) : (state.ball.velocity.x > kInboundEpsilon);
}

void advance_ai_plan_timers(whacker::app::RuntimeAiState& ai_state) {
    ++ai_state.runtime_step_counter;
    if (ai_state.plan.replan_cooldown_steps > 0) {
        --ai_state.plan.replan_cooldown_steps;
    }
}

AiReplanPolicy evaluate_replan_policy(
    const whacker::app::RuntimeAiState& ai_state,
    const std::uint64_t state_signature,
    const bool inbound) {
    const auto& plan = ai_state.plan;
    const bool signature_changed = plan.has_plan && (plan.state_signature != state_signature);
    const bool inbound_flipped = plan.has_plan && (plan.ball_was_inbound != inbound);
    const bool plan_expired = plan.has_plan && (ai_state.runtime_step_counter > plan.valid_until_step);
    const bool confidence_low_near_contact =
        plan.has_plan &&
        (plan.confidence < kMinConfidenceReplan) &&
        (plan.intercept_time_s <= kLowConfidenceNearContactWindowS);

    AiReplanPolicy policy {};
    policy.forced_replan = !plan.has_plan || signature_changed || inbound_flipped || plan_expired || confidence_low_near_contact;
    const bool cooldown_elapsed = plan.has_plan && (plan.replan_cooldown_steps <= 0);
    policy.optional_replan = !policy.forced_replan && cooldown_elapsed;
    policy.should_replan = policy.forced_replan || policy.optional_replan;
    return policy;
}

bool should_keep_existing_plan(
    const whacker::app::RuntimeAiPlanState& plan,
    const whacker::app::AiDecision& proposed,
    const bool forced_replan) {
    if (forced_replan || !plan.has_plan || !proposed.valid) {
        return false;
    }
    if (plan.ball_was_inbound != proposed.inbound) {
        return false;
    }
    return proposed.score <= (plan.decision_score + kPlanHysteresisThreshold);
}

void maybe_replan_ai_target(
    const whacker::sim::Simulation& simulation,
    const bool for_left_paddle,
    whacker::app::RuntimeAiState& ai_state,
    const std::uint64_t state_signature,
    const bool inbound,
    const bool ambient_mode) {
    const AiReplanPolicy policy = evaluate_replan_policy(ai_state, state_signature, inbound);
    if (!policy.should_replan) {
        return;
    }

    const whacker::app::AiDecision proposed = whacker::app::plan_ai_decision(
        simulation,
        for_left_paddle,
        ai_state,
        ai_state.decision_counter,
        ambient_mode,
        ambient_mode ? kAmbientPlannerConfig : kPlayPlannerConfig);
    ++ai_state.decision_counter;

    auto& plan = ai_state.plan;
    if (should_keep_existing_plan(plan, proposed, policy.forced_replan)) {
        plan.replan_cooldown_steps = std::max(plan.replan_cooldown_steps, 1);
        return;
    }

    whacker::app::write_runtime_ai_plan_from_decision(ai_state, proposed, state_signature, inbound);
}

void apply_center_fallback_target(
    whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config) {
    const float min_y = config.paddle_half_height;
    const float max_y = config.court_height - config.paddle_half_height;
    paddle.target_y = clampf(0.5f * config.court_height, min_y, max_y);
    paddle.feedforward_velocity_y = 0.0f;
}

void set_ai_target_from_plan(
    whacker::sim::Simulation& simulation,
    const bool for_left_paddle,
    whacker::app::RuntimeAiState& ai_state,
    const float dt,
    const bool ambient_mode) {
    auto& state = simulation.mutable_state();
    auto& paddle = for_left_paddle ? state.left : state.right;
    auto& plan = ai_state.plan;
    const auto& config = simulation.config();

    const std::uint64_t state_signature = whacker::app::compute_ai_replan_signature(state, for_left_paddle);
    const bool inbound = ball_moving_toward_paddle(state, for_left_paddle);

    advance_ai_plan_timers(ai_state);
    maybe_replan_ai_target(simulation, for_left_paddle, ai_state, state_signature, inbound, ambient_mode);

    if (!plan.has_plan) {
        apply_center_fallback_target(paddle, config);
        return;
    }

    plan.intercept_time_s = inbound ? std::max(0.0f, plan.intercept_time_s - dt) : 0.0f;

    plan.state_signature = state_signature;
    plan.ball_was_inbound = inbound;
    const whacker::app::AiDecision active_decision = whacker::app::runtime_ai_decision_from_plan(plan);
    whacker::app::apply_ai_decision(
        paddle,
        config,
        active_decision,
        inbound,
        plan.intercept_time_s,
        ambient_mode);
}

}  // namespace

namespace whacker::app {

void reset_runtime_ai_plan(RuntimeAiState& ai_state) {
    ai_state.plan = {};
}

void set_paddle_execution_full(whacker::sim::PaddleState& paddle) {
    paddle.power_scale = 1.0f;
    paddle.technical_scale = 1.0f;
    paddle.spin_scale = 1.0f;
}

void set_paddle_execution_from_skills(
    whacker::sim::PaddleState& paddle,
    const whacker::progression::SkillState& skills) {
    constexpr float kPowerFloor = 0.10f;
    constexpr float kTechnicalFloor = 0.00f;
    constexpr float kSpinFloor = 0.00f;
    paddle.power_scale = skill_scale_with_floor(skills.power, kPowerFloor);
    paddle.technical_scale = skill_scale_with_floor(skills.edge, kTechnicalFloor);
    paddle.spin_scale = skill_scale_with_floor(skills.spin_inject, kSpinFloor);
}

void sync_runtime_ai_style(RuntimeAiState& ai_state, const AiStyle style) {
    if (ai_state.initialized && ai_state.style == style) {
        return;
    }
    const AiStyleProfile& archetype = ai_style_profile(style);
    ai_state.initialized = true;
    ai_state.style = style;
    ai_state.skills = archetype.seed_skills;
    ai_state.profile = archetype.profile;
    whacker::progression::clamp_skills_to_profile(ai_state.skills, ai_state.profile);
    ai_state.decision_counter = 0ULL;
    ai_state.runtime_step_counter = 0;
    reset_runtime_ai_plan(ai_state);
}

void update_targets_for_play(
    GLFWwindow* window,
    whacker::sim::Simulation& simulation,
    const MatchOptions& options,
    const ControlBindings& controls,
    RuntimeAiState& left_ai_state,
    RuntimeAiState& right_ai_state,
    const float dt,
    const PlayControlOverrides* overrides) {
    auto& state = simulation.mutable_state();
    const auto& config = simulation.config();

    PaddleMode left_mode = options.left_mode;
    PaddleMode right_mode = options.right_mode;
    if (overrides != nullptr && overrides->force_modes) {
        left_mode = overrides->left_mode;
        right_mode = overrides->right_mode;
    }

    whacker::progression::SkillState left_skills = options.left_paddle_skills;
    whacker::progression::SkillState right_skills = options.right_paddle_skills;
    if (overrides != nullptr && overrides->override_left_skills) {
        left_skills = overrides->left_skills;
    }
    if (overrides != nullptr && overrides->override_right_skills) {
        right_skills = overrides->right_skills;
    }
    whacker::progression::clamp_skills(left_skills);
    whacker::progression::clamp_skills(right_skills);

    sync_runtime_ai_style(left_ai_state, style_for_skills(left_skills));
    sync_runtime_ai_style(right_ai_state, style_for_skills(right_skills));
    left_ai_state.skills = left_skills;
    right_ai_state.skills = right_skills;

    // Style stats still apply to both paddles so execution behavior remains shared.
    set_paddle_execution_from_skills(state.left, left_ai_state.skills);
    set_paddle_execution_from_skills(state.right, right_ai_state.skills);

    if (left_mode == PaddleMode::Human) {
        const bool left_up = glfwGetKey(window, controls.p1_up) == GLFW_PRESS;
        const bool left_down = glfwGetKey(window, controls.p1_down) == GLFW_PRESS;
        set_human_target(state.left, config, left_up, left_down, dt);
        reset_runtime_ai_plan(left_ai_state);
    } else {
        set_ai_target_from_plan(simulation, true, left_ai_state, dt, false);
    }

    if (right_mode == PaddleMode::Human) {
        const bool right_up = glfwGetKey(window, controls.p2_up) == GLFW_PRESS;
        const bool right_down = glfwGetKey(window, controls.p2_down) == GLFW_PRESS;
        set_human_target(state.right, config, right_up, right_down, dt);
        reset_runtime_ai_plan(right_ai_state);
    } else {
        set_ai_target_from_plan(simulation, false, right_ai_state, dt, false);
    }
}

void update_targets_for_ambient(
    whacker::sim::Simulation& simulation,
    const MatchOptions& options,
    RuntimeAiState& left_ai_state,
    RuntimeAiState& right_ai_state,
    const float dt) {
    auto& state = simulation.mutable_state();
    whacker::progression::SkillState left_skills = options.left_paddle_skills;
    whacker::progression::SkillState right_skills = options.right_paddle_skills;
    whacker::progression::clamp_skills(left_skills);
    whacker::progression::clamp_skills(right_skills);

    sync_runtime_ai_style(left_ai_state, style_for_skills(left_skills));
    sync_runtime_ai_style(right_ai_state, style_for_skills(right_skills));
    left_ai_state.skills = left_skills;
    right_ai_state.skills = right_skills;

    set_paddle_execution_full(state.left);
    set_paddle_execution_full(state.right);
    set_ai_target_from_plan(simulation, true, left_ai_state, dt, true);
    set_ai_target_from_plan(simulation, false, right_ai_state, dt, true);
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
