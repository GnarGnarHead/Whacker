#include "runtime_phase_drivers.hpp"

#ifdef WHACKER_HAS_GLFW

#include <cassert>

#include "runtime_input_phase.hpp"
#include "runtime_step_input.hpp"
#include "runtime_step_phase.hpp"

namespace whacker::app {

namespace {

void reset_state_change_lockout_and_ai_plans(
    RuntimeUpdatePhaseContext& context,
    double& menu_input_lockout,
    const double menu_input_lockout_seconds) {
    context.left_ai_state.plan = {};
    context.right_ai_state.plan = {};
    menu_input_lockout = menu_input_lockout_seconds;
}

void apply_state_change_side_effects(
    RuntimeUpdatePhaseContext& context,
    const AppState previous_app_state,
    double& menu_input_lockout,
    const double menu_input_lockout_seconds) {
    if (previous_app_state != context.app_state) {
        reset_state_change_lockout_and_ai_plans(context, menu_input_lockout, menu_input_lockout_seconds);
    }
}

bool try_start_authored_visual_transition(
    RuntimeUpdatePhaseContext& context,
    const double now_seconds,
    double& menu_input_lockout,
    const double menu_input_lockout_seconds) {
    RuntimeAuthoredTransitionRequest& request = context.authored_transition_request;
    if (!request.armed) {
        return false;
    }
    if (request.from_state == AppState::StoryScene && request.has_from_story_scene) {
        context.story_scene_state = request.from_story_scene;
    }
    context.app_state = request.from_state;
    reset_state_change_lockout_and_ai_plans(context, menu_input_lockout, menu_input_lockout_seconds);
    begin_visual_transition_for_authored_request(context.visual_transition, request, now_seconds);
    clear_authored_transition_request(request);
    return true;
}

}  // namespace

void run_runtime_update_phases(
    RuntimeUpdatePhaseContext& context,
    const double now,
    double& accumulator,
    double& menu_input_lockout,
    const double menu_input_lockout_seconds,
    const int story_official_games_to_win,
    RuntimeStorySaveExistsCache* story_save_cache) {
    if (context.visual_transition.active) {
        accumulator = 0.0;
        advance_visual_transition(
            context.visual_transition,
            context.app_state,
            context.story_scene_state,
            now);
        return;
    }

    const AppState app_state_before_input = context.app_state;

    RuntimeInputPhaseArgs input_args {
        context.window,
        context.edge_state,
        context.app_state,
        context.pause_return_state,
        context.show_dev_info,
        context.ai_controls_player_paddle,
        context.options,
        context.controls,
        context.audio_settings,
        context.audio_engine,
        context.main_menu_state,
        context.options_menu_state,
        context.pause_menu_state,
        context.menu_state,
        context.paddle_tuning_state,
        context.story_menu_state,
        context.story_intro_state,
        context.story_scene_state,
        context.story_hub_state,
        context.story_runtime,
        context.authored_transition_request,
        context.match_flow,
        context.simulation,
        context.rng,
        menu_input_lockout,
        story_official_games_to_win,
        story_save_cache};
    handle_runtime_input_phase(input_args);

    apply_state_change_side_effects(
        context,
        app_state_before_input,
        menu_input_lockout,
        menu_input_lockout_seconds);
    if (try_start_authored_visual_transition(
            context,
            now,
            menu_input_lockout,
            menu_input_lockout_seconds)) {
        accumulator = 0.0;
        return;
    }

    const AppState app_state_before_step = context.app_state;

    const RuntimeStepInputSnapshot step_input = sample_runtime_step_input(context.window);
    RuntimeStepPhaseArgs step_args {
        context.window,
        now,
        accumulator,
        step_input,
        context.app_state,
        context.ai_controls_player_paddle,
        context.options,
        context.controls,
        context.story_runtime,
        context.story_hub_state,
        context.story_intro_state,
        context.story_scene_state,
        context.authored_transition_request,
        context.match_flow,
        context.left_ai_state,
        context.right_ai_state,
        context.simulation,
        context.rng,
        context.audio_engine,
        context.type_blip_cooldown,
        context.type_blip_pattern_step,
        story_official_games_to_win};
    handle_runtime_step_phase(step_args);

    apply_state_change_side_effects(
        context,
        app_state_before_step,
        menu_input_lockout,
        menu_input_lockout_seconds);
    if (try_start_authored_visual_transition(
            context,
            now,
            menu_input_lockout,
            menu_input_lockout_seconds)) {
        accumulator = 0.0;
    }
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
