#pragma once

#ifdef WHACKER_HAS_GLFW

#include <cstdint>
#include <random>

#include "app_types.hpp"
#include "audio_engine.hpp"
#include "match_flow.hpp"
#include "paddle_tuning.hpp"
#include "runtime_phase_drivers.hpp"
#include "runtime_render_drivers.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

struct GLFWwindow;

namespace whacker::app {

struct RuntimeLoopState {
    AppState app_state = AppState::MainMenu;
    AppState pause_return_state = AppState::Playing;
    MatchOptions options {};
    ControlBindings controls {};
    AudioSettings audio_settings {};
    AudioEngine audio_engine {};
    MainMenuState main_menu_state {};
    OptionsMenuState options_menu_state {};
    PauseMenuState pause_menu_state {};
    MenuState menu_state {};
    PaddleTuningState paddle_tuning_state {};
    StoryMenuState story_menu_state {};
    StoryIntroState story_intro_state {};
    StorySceneState story_scene_state {};
    StoryHubState story_hub_state {};
    StoryRuntimeState story_runtime {};
    RuntimeVisualTransitionState visual_transition {};
    RuntimeAuthoredTransitionRequest authored_transition_request {};
    MatchFlowState match_flow {};
    RuntimeAiState left_ai_state {};
    RuntimeAiState right_ai_state {};
    KeyEdgeState edge_state {};
    bool show_dev_info = false;
    bool ai_controls_player_paddle = false;
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;
    std::mt19937_64 rng;

    explicit RuntimeLoopState(std::uint64_t rng_seed);
};

void initialize_runtime_loop_state(RuntimeLoopState& loop_state);

RuntimeUpdatePhaseContext make_runtime_update_phase_context(
    GLFWwindow* window,
    RuntimeLoopState& loop_state,
    whacker::sim::Simulation& simulation);

RuntimeRenderDriverContext make_runtime_render_driver_context(
    GLFWwindow* window,
    RuntimeLoopState& loop_state,
    const whacker::sim::Simulation& simulation);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
