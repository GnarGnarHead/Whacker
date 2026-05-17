#pragma once

#include "app_types.hpp"
#include "menu_input.hpp"
#include "sim/physics.hpp"

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

void reset_runtime_ai_plan(RuntimeAiState& ai_state);

void set_paddle_execution_full(whacker::sim::PaddleState& paddle);
void set_paddle_execution_from_skills(
    whacker::sim::PaddleState& paddle,
    const whacker::progression::SkillState& skills);

void sync_runtime_ai_style(RuntimeAiState& ai_state, AiStyle style);

void update_targets_for_play(
    GLFWwindow* window,
    whacker::sim::Simulation& simulation,
    const MatchOptions& options,
    const ControlBindings& controls,
    RuntimeAiState& left_ai_state,
    RuntimeAiState& right_ai_state,
    float dt,
    const PlayControlOverrides* overrides = nullptr);

void update_targets_for_ambient(
    whacker::sim::Simulation& simulation,
    const MatchOptions& options,
    RuntimeAiState& left_ai_state,
    RuntimeAiState& right_ai_state,
    float dt);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
