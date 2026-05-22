#pragma once

#include "app_types.hpp"
#include "control_plan.hpp"
#include "sim/physics.hpp"

namespace whacker::app {

void reset_runtime_ai_plan(RuntimeAiState& ai_state);

void set_paddle_execution_full(whacker::sim::PaddleState& paddle);
void set_paddle_execution_from_skills(
    whacker::sim::PaddleState& paddle,
    const whacker::progression::SkillState& skills);

void sync_runtime_ai_style(RuntimeAiState& ai_state, AiStyle style);

void update_targets_for_play(
    whacker::sim::Simulation& simulation,
    const MatchOptions& options,
    RuntimeAiState& left_ai_state,
    RuntimeAiState& right_ai_state,
    float dt,
    const MatchControlPlan& control_plan,
    InputSlotAxes input_axes,
    const PlayControlOverrides* overrides = nullptr);

void update_targets_for_ambient(
    whacker::sim::Simulation& simulation,
    const MatchOptions& options,
    RuntimeAiState& left_ai_state,
    RuntimeAiState& right_ai_state,
    float dt);

}  // namespace whacker::app
