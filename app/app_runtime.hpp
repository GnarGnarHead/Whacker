#pragma once

#include "sim/physics.hpp"

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

int run_app_loop(GLFWwindow* window, whacker::sim::Simulation& simulation);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
