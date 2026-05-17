#pragma once

#include "app_types.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

using IntNameFn = const char* (*)(int);
using ModeNameFn = const char* (*)(PaddleMode);
using StyleNameFn = const char* (*)(AiStyle);
using MatchKindNameFn = const char* (*)(StoryMatchKind);
using IntroPhaseNameFn = const char* (*)(StoryIntroPhase);

void update_window_title(
    GLFWwindow* window,
    const whacker::sim::Simulation& simulation,
    const MatchOptions& options,
    const OptionsMenuState& options_menu_state,
    const MainMenuState& main_menu_state,
    const MenuState& menu_state,
    const StoryMenuState& story_menu_state,
    const StoryIntroState& story_intro_state,
    const StoryRuntimeState& story_runtime,
    const StoryHubState& story_hub_state,
    AppState app_state,
    IntNameFn main_menu_row_name_fn,
    IntNameFn options_menu_row_name_fn,
    IntNameFn quick_row_name_fn,
    IntNameFn story_menu_row_name_fn,
    IntroPhaseNameFn story_intro_phase_name_fn,
    IntNameFn story_hub_row_name_fn,
    ModeNameFn mode_name_fn,
    StyleNameFn style_name_fn,
    MatchKindNameFn match_kind_name_fn);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
