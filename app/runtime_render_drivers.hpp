#pragma once

#ifdef WHACKER_HAS_GLFW

#include "runtime_render_phase.hpp"
#include "runtime_visual_transition.hpp"
#include "window_title.hpp"

struct GLFWwindow;

namespace whacker::app {

struct RuntimeRenderDriverContext {
    GLFWwindow* window;
    const whacker::sim::Simulation& simulation;
    const MatchFlowState& match_flow;
    bool& show_dev_info;
    bool& ai_controls_player_paddle;
    AppState& app_state;
    AppState& pause_return_state;
    const MainMenuState& main_menu_state;
    const OptionsMenuState& options_menu_state;
    const PauseMenuState& pause_menu_state;
    const MenuState& menu_state;
    const PaddleTuningState& paddle_tuning_state;
    const StoryMenuState& story_menu_state;
    const StoryIntroState& story_intro_state;
    const StorySceneState& story_scene_state;
    const StoryHubState& story_hub_state;
    const StoryRuntimeState& story_runtime;
    const RuntimeVisualTransitionState& visual_transition;
    const MatchOptions& options;
    const ControlBindings& controls;
    const AudioSettings& audio_settings;
    IntNameFn main_menu_row_name_fn;
    IntNameFn options_menu_row_name_fn;
    IntNameFn quick_row_name_fn;
    IntNameFn story_menu_row_name_fn;
    IntroPhaseNameFn story_intro_phase_name_fn;
    IntNameFn story_hub_row_name_fn;
    ModeNameFn mode_name_fn;
    StyleNameFn style_name_fn;
    MatchKindNameFn story_match_kind_name_fn;
};

void run_runtime_render_phases(
    RuntimeRenderDriverContext& context,
    double& title_cooldown,
    RuntimeStorySaveExistsCache* story_save_cache = nullptr);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
