#pragma once

#include "app_types.hpp"
#include "render_context.hpp"
#include "ui_state.hpp"

namespace whacker::app {

void render_menu_overlay(const RenderContext& context, const MatchOptions& options, const MenuState& menu_state);

}  // namespace whacker::app
