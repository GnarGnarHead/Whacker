#pragma once

#include <cstddef>

#include "sdl_runtime_state.hpp"
#include "sim/physics.hpp"

namespace whacker::app {

void initialize_sdl_runtime_audio(SdlRuntimeState& runtime);
void shutdown_sdl_runtime_audio(SdlRuntimeState& runtime);
void apply_sdl_runtime_audio_settings(SdlRuntimeState& runtime);

void play_menu_move_sound(SdlRuntimeState& runtime);
void play_menu_confirm_sound(SdlRuntimeState& runtime);

void tick_story_typewriter_audio(SdlRuntimeState& runtime, float dt_seconds);
void route_story_typewriter_audio(
    SdlRuntimeState& runtime,
    bool dialogue_writing,
    std::size_t visible_before,
    std::size_t visible_after);

void play_match_opening_countdown_cue(SdlRuntimeState& runtime);

void route_step_audio_events(
    SdlRuntimeState& runtime,
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    whacker::sim::ScoreEvent score_event,
    const whacker::sim::SimulationConfig& config);

}  // namespace whacker::app
