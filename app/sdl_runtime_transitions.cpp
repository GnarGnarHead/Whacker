#include "sdl_runtime_transitions.hpp"

#include <algorithm>

#include "match_end_flow.hpp"
#include "match_flow.hpp"
#include "platform_sdl.hpp"
#include "runtime_transitions.hpp"
#include "story_match.hpp"
#include "story_runtime_invariants.hpp"
#include "story_save.hpp"
#include "story_scene.hpp"

namespace whacker::app {

int story_official_games_to_win() {
    return std::max(1, story_match_policy_for_kind(StoryMatchKind::Official).games_to_win);
}

void reset_pause_menu(SdlRuntimeState& runtime) {
    runtime.pause_return_state = AppState::Playing;
    runtime.pause_menu.selected_row = PauseMenuRowResume;
    runtime.pause_menu.confirm_forfeit = false;
    runtime.pause_menu.confirm_selected = 0;
}

void return_to_main_menu(SdlRuntimeState& runtime) {
    runtime.app_state = AppState::MainMenu;
    runtime.main_menu_feedback.clear();
    runtime.story_menu_feedback.clear();
    runtime.story_hub.feedback_line_1.clear();
    runtime.story_hub.feedback_line_2.clear();
    runtime.paddle_tuning.active = false;
    runtime.story_intro = StoryIntroState {};
    clear_story_scene(runtime.story_scene);
    clear_story_runtime_scene_pending_flags(runtime.story_runtime);
    clear_authored_transition_request(runtime.authored_transition_request);
    runtime.visual_transition = RuntimeVisualTransitionState {};
    reset_match_flow(runtime.match_flow);
    reset_story_match_tracking(runtime.story_runtime);
    runtime.story_runtime.active_match = StoryMatchKind::None;
    reset_pause_menu(runtime);
    runtime.accumulator = 0.0;
}

void enter_quick_match_setup(SdlRuntimeState& runtime) {
    runtime.quick_menu.selected_row = MenuRowP1;
    runtime.app_state = AppState::QuickMatchSetup;
    runtime.main_menu_feedback.clear();
    runtime.accumulator = 0.0;
}

void enter_options_menu(SdlRuntimeState& runtime) {
    runtime.options_menu.waiting_for_key = false;
    runtime.app_state = AppState::OptionsMenu;
    runtime.main_menu_feedback.clear();
    runtime.accumulator = 0.0;
}

void enter_story_menu(SdlRuntimeState& runtime) {
    runtime.story_menu.selected_row = story_save_exists() ? StoryMenuRowContinue : StoryMenuRowNewCareer;
    runtime.story_menu.confirm_overwrite = false;
    runtime.story_menu.confirm_selected = 0;
    runtime.story_menu_feedback.clear();
    runtime.app_state = AppState::StoryMenu;
    runtime.main_menu_feedback.clear();
    runtime.accumulator = 0.0;
}

void start_quick_match(SdlRuntimeState& runtime, whacker::sim::Simulation& simulation) {
    simulation.reset();
    const bool opening_serve_to_right = randomize_opening_serve(simulation, runtime.rng);
    start_match_flow(runtime.match_flow, ActiveMatchMode::Quick, opening_serve_to_right, true);
    start_match_opening_countdown(runtime.match_flow, simulation);
    runtime.main_menu_feedback.clear();
    reset_pause_menu(runtime);
    runtime.app_state = AppState::Playing;
}

void apply_main_menu_result(
    const MainMenuActionResult result,
    SdlRuntimeState& runtime,
    SdlPlatform& platform) {
    switch (result) {
        case MainMenuActionResult::None:
            return;
        case MainMenuActionResult::Story:
            enter_story_menu(runtime);
            return;
        case MainMenuActionResult::Quick:
            enter_quick_match_setup(runtime);
            return;
        case MainMenuActionResult::Options:
            enter_options_menu(runtime);
            return;
        case MainMenuActionResult::Quit:
            platform.request_close();
            return;
    }
}

void finish_active_or_quick_match(
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation,
    const StoryMatchEndReason end_reason) {
    end_active_or_quick_match(
        runtime.story_runtime,
        runtime.story_hub,
        runtime.match_flow,
        simulation,
        runtime.story_scene,
        runtime.authored_transition_request,
        runtime.app_state,
        end_reason,
        story_official_games_to_win(),
        save_story_career);
}

}  // namespace whacker::app
