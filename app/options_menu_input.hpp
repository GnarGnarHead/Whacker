#pragma once

#include "audio_engine.hpp"
#include "menu_input.hpp"
#include "ui_state.hpp"

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

int binding_value(const ControlBindings& bindings, int row);
bool options_row_is_binding(int row);
bool options_row_is_volume(int row);
bool options_row_is_mute(int row);
int audio_value(const AudioSettings& settings, int row);
bool audio_toggle_value(const AudioSettings& settings, int row);

void handle_options_menu_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    OptionsMenuState& options_menu_state,
    ControlBindings& controls,
    AudioSettings& audio_settings,
    AppState& app_state,
    bool& changed_bindings,
    bool& changed_audio_settings);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
