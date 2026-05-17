#pragma once

#ifdef WHACKER_HAS_GLFW

#include "runtime_visual_transition.hpp"

struct GLFWwindow;

namespace whacker::app {

void render_visual_transition_overlay(
    GLFWwindow* window,
    const RuntimeVisualTransitionState& transition);

void release_visual_transition_render_resources();

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW

