#pragma once

#include <string>

#include "sim/physics.hpp"

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

void render_scene(GLFWwindow* window, const whacker::sim::Simulation& simulation, bool ball_visible = true);
void render_hud(GLFWwindow* window, const whacker::sim::Simulation& simulation);
void render_play_center_message(GLFWwindow* window, const std::string& message);
void render_dev_overlay(
    GLFWwindow* window,
    const whacker::sim::Simulation& simulation,
    bool ai_controls_player_paddle);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
