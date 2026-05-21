#pragma once

#include <string>

#include "sim/physics.hpp"

namespace whacker::app {

class SdlPlatform;

void render_scene(SdlPlatform* platform, const whacker::sim::Simulation& simulation, bool ball_visible = true);
void render_hud(SdlPlatform* platform, const whacker::sim::Simulation& simulation);
void render_play_center_message(SdlPlatform* platform, const std::string& message);
void render_dev_overlay(
    SdlPlatform* platform,
    const whacker::sim::Simulation& simulation,
    bool ai_controls_player_paddle);

}  // namespace whacker::app
