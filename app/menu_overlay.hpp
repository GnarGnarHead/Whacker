#pragma once

#include <string>

#include "match_exit_policy.hpp"
#include "render_context.hpp"
#include "ui_state.hpp"

namespace whacker::app {

using RowNameFn = const char* (*)(int);
using OptionsRowNameFn = const char* (*)(OptionsMenuSection section, int row);
using OptionsValueLabelFn = std::string (*)(const OptionsMenuState& menu_state, int row, const void* context);

void render_main_menu_overlay(
    const RenderContext& context,
    const MainMenuState& menu_state,
    RowNameFn row_name_fn,
    const std::string& status_message = {});

void render_pause_overlay(
    const RenderContext& context,
    const PauseMenuState& pause_menu_state,
    const MatchExitPolicy& exit_policy);

void render_options_menu_overlay(
    const RenderContext& context,
    const OptionsMenuState& menu_state,
    OptionsRowNameFn row_name_fn,
    OptionsValueLabelFn value_label_fn,
    const void* value_label_context);

}  // namespace whacker::app
