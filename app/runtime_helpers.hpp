#pragma once

#ifdef WHACKER_HAS_GLFW

#include "app_types.hpp"
#include "audio_engine.hpp"
#include "sim/physics.hpp"
#include "story_intro.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

void track_intro_contact_usage(
    StoryIntroState& story_intro_state,
    const whacker::sim::SimulationConfig& config,
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after);

const char* mode_name(PaddleMode mode);
const char* ai_style_name(AiStyle style);
const char* row_name(int row);
const char* main_menu_row_name(int row);
const char* options_menu_row_name(int row);
const char* story_menu_row_name(int row);
const char* story_hub_row_name(int row);
const char* story_match_kind_name(StoryMatchKind kind);
const char* story_intro_phase_name(StoryIntroPhase phase);

bool detect_wall_bounce(
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const whacker::sim::SimulationConfig& config);

PaddleHitAudioParams build_paddle_hit_audio_params(
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const whacker::sim::SimulationConfig& config);

WallHitAudioParams build_wall_hit_audio_params(
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const whacker::sim::SimulationConfig& config);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
