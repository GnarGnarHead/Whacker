#pragma once

#ifdef WHACKER_HAS_GLFW

#include "paddle_tuning.hpp"

struct GLFWwindow;

namespace whacker::app {

void render_paddle_tuning_overlay(GLFWwindow* window, const PaddleTuningState& tuning_state);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
