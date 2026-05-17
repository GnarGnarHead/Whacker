#pragma once

#ifdef WHACKER_HAS_GLFW

#include "app_types.hpp"
#include "audio_engine.hpp"
#include "match_exit_policy.hpp"
#include "match_flow.hpp"
#include "paddle_tuning.hpp"
#include "runtime_story_save_cache.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

struct GLFWwindow;

namespace whacker::app {

void render_runtime_frame(
    GLFWwindow* window,
    const whacker::sim::Simulation& simulation,
    const MatchFlowState& match_flow,
    bool show_dev_info,
    bool ai_controls_player_paddle,
    AppState app_state,
    const MainMenuState& main_menu_state,
    const OptionsMenuState& options_menu_state,
    const PauseMenuState& pause_menu_state,
    const MenuState& menu_state,
    const PaddleTuningState& paddle_tuning_state,
    const StoryMenuState& story_menu_state,
    const StoryIntroState& story_intro_state,
    const StorySceneState& story_scene_state,
    const StoryHubState& story_hub_state,
    const StoryRuntimeState& story_runtime,
    const MatchOptions& options,
    const ControlBindings& controls,
    const AudioSettings& audio_settings,
    const MatchExitPolicy* pause_exit_policy,
    RuntimeStorySaveExistsCache* story_save_cache = nullptr);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
