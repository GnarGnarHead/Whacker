#pragma once

#include <string>

#include "render_context.hpp"
#include "ui_state.hpp"

#ifdef WHACKER_HAS_GLFW
#include "audio_engine.hpp"
#include "match_exit_policy.hpp"
#include "menu_input.hpp"

struct GLFWwindow;
#endif

namespace whacker::app {

using RowNameFn = const char* (*)(int);

void render_main_menu_overlay(
    const RenderContext& context,
    const MainMenuState& menu_state,
    RowNameFn row_name_fn,
    const std::string& status_message = {});

#ifdef WHACKER_HAS_GLFW

using KeyNameFn = const char* (*)(int);
using BindingValueFn = int (*)(const ControlBindings&, int);

void render_main_menu_overlay(
    GLFWwindow* window,
    const MainMenuState& menu_state,
    RowNameFn row_name_fn);

void render_options_menu_overlay(
    GLFWwindow* window,
    const OptionsMenuState& menu_state,
    const ControlBindings& controls,
    const AudioSettings& audio_settings,
    RowNameFn row_name_fn,
    KeyNameFn key_name_fn,
    BindingValueFn binding_value_fn);

void render_pause_overlay(
    GLFWwindow* window,
    const PauseMenuState& pause_menu_state,
    const MatchExitPolicy& exit_policy);

#endif  // WHACKER_HAS_GLFW

}  // namespace whacker::app
