#pragma once

#include "app_types.hpp"
#include "ui_state.hpp"

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

void render_menu_overlay(GLFWwindow* window, const MatchOptions& options, const MenuState& menu_state);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
