#pragma once

#include <random>
#include <string>

#include "app_types.hpp"
#include "match_flow.hpp"
#include "menu_input.hpp"
#include "runtime_visual_transition.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_runtime.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

using StoryLoadCareerFn = bool (*)(StoryCareerData&, std::string*);
using KeyToNameCharFn = bool (*)(int, char&);
using TrimCopyFn = std::string (*)(const std::string&);

void handle_story_intro_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchOptions& options,
    const ControlBindings& controls,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    AppState& app_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    KeyToNameCharFn key_to_name_char_fn,
    TrimCopyFn trim_copy_fn,
    StorySanitizeNameFn sanitize_name_fn,
    StorySaveCareerCallback save_career_fn);

void handle_story_menu_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    StoryMenuState& story_menu_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchOptions& options,
    const ControlBindings& controls,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    AppState& app_state,
    bool has_save,
    StoryLoadCareerFn load_career_fn,
    StoryResetCareerFn reset_career_fn);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
