#pragma once

#include <cstdint>
#include <random>
#include <string>

#include "app_types.hpp"
#include "audio_engine.hpp"
#include "match_flow.hpp"
#include "menu_input.hpp"
#include "paddle_tuning.hpp"
#include "runtime_visual_transition.hpp"
#include "sdl_input.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct SdlRuntimeState {
    AppState app_state = AppState::MainMenu;
    AppState pause_return_state = AppState::Playing;
    MainMenuState main_menu {};
    MenuState quick_menu {};
    OptionsMenuState options_menu {};
    PauseMenuState pause_menu {};
    MatchOptions options {};
    MatchFlowState match_flow {};
    PaddleTuningState paddle_tuning {};
    StoryMenuState story_menu {};
    StoryHubState story_hub {};
    StoryIntroState story_intro {};
    StorySceneState story_scene {};
    StoryRuntimeState story_runtime {};
    RuntimeVisualTransitionState visual_transition {};
    RuntimeAuthoredTransitionRequest authored_transition_request {};
    RuntimeAiState left_ai {};
    RuntimeAiState right_ai {};
    AudioSettings audio_settings {};
    AudioEngine audio_engine {};
    ControlBindings controls {};
    std::string main_menu_feedback {};
    std::string story_menu_feedback {};
    SdlInput input {};
    std::mt19937_64 rng {0x575841434B455252ULL};
    double previous_time = 0.0;
    double accumulator = 0.0;
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0;
};

void initialize_sdl_runtime_state(SdlRuntimeState& runtime);
void shutdown_sdl_runtime_state(SdlRuntimeState& runtime);
void sync_controls_from_action_bindings(SdlRuntimeState& runtime);
void persist_runtime_menu_settings(const SdlRuntimeState& runtime);

}  // namespace whacker::app
