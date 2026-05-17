#pragma once

#ifdef WHACKER_HAS_GLFW

#include "paddle_tuning.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

struct GLFWwindow;

namespace whacker::app {

bool handle_runtime_escape_key(
    GLFWwindow* window,
    AppState& app_state,
    AppState& pause_return_state,
    StoryMenuState& story_menu_state,
    OptionsMenuState& options_menu_state,
    PauseMenuState& pause_menu_state,
    StorySceneState& story_scene_state,
    PaddleTuningState& paddle_tuning_state,
    StoryRuntimeState& story_runtime);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
