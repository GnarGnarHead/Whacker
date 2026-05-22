#pragma once

#include "main_menu_actions.hpp"
#include "sdl_runtime_state.hpp"
#include "sim/physics.hpp"
#include "story_match.hpp"

namespace whacker::app {

class SdlPlatform;

int story_official_games_to_win();
void sync_runtime_app_state(SdlRuntimeState& runtime);
void push_runtime_screen(SdlRuntimeState& runtime, Screen screen);
bool pop_runtime_screen(SdlRuntimeState& runtime);
void replace_runtime_screen(SdlRuntimeState& runtime, Screen screen);
void reset_runtime_to_root(SdlRuntimeState& runtime, Screen screen);
Screen runtime_active_screen(const SdlRuntimeState& runtime);
void reset_pause_menu(SdlRuntimeState& runtime);
void return_to_main_menu(SdlRuntimeState& runtime);
void enter_quick_match_setup(SdlRuntimeState& runtime);
void enter_options_menu(SdlRuntimeState& runtime);
void enter_story_menu(SdlRuntimeState& runtime);
void start_quick_match(SdlRuntimeState& runtime, whacker::sim::Simulation& simulation);
void apply_main_menu_result(MainMenuActionResult result, SdlRuntimeState& runtime, SdlPlatform& platform);
void finish_active_or_quick_match(
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation,
    StoryMatchEndReason end_reason);

}  // namespace whacker::app
