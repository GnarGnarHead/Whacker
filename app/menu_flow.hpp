#pragma once

#include <random>

#include "app_types.hpp"
#include "match_flow.hpp"
#include "menu_input.hpp"
#include "sim/physics.hpp"
#include "story_runtime.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

void handle_main_menu_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    MainMenuState& main_menu_state,
    MenuState& quick_menu_state,
    StoryMenuState& story_menu_state,
    OptionsMenuState& options_menu_state,
    const ControlBindings& controls,
    AppState& app_state);

void set_menu_row_option(MatchOptions& options, const MenuState& menu_state, int direction);

void handle_menu_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    MenuState& menu_state,
    MatchOptions& options,
    const ControlBindings& controls,
    MatchFlowState& match_flow,
    AppState& app_state,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng);

void handle_story_hub_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchOptions& options,
    const ControlBindings& controls,
    MatchFlowState& match_flow,
    AppState& app_state,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    StorySaveCareerCallback save_career_fn = nullptr);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
