#pragma once

#include <string>

#include "render_context.hpp"
#include "sim/physics.hpp"

namespace whacker::app {

void render_scene(const RenderContext& context, const whacker::sim::Simulation& simulation, bool ball_visible = true);
void render_hud(const RenderContext& context, const whacker::sim::Simulation& simulation);
void render_play_center_message(const RenderContext& context, const std::string& message);
void render_dev_overlay(
    const RenderContext& context,
    const whacker::sim::Simulation& simulation,
    bool ai_controls_player_paddle);

}  // namespace whacker::app
