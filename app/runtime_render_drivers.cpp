#include "runtime_render_drivers.hpp"

#ifdef WHACKER_HAS_GLFW

#include "runtime_transitions.hpp"
#include "runtime_visual_transition_render.hpp"

namespace whacker::app {

namespace {

constexpr double kWindowTitleUpdateSeconds = 0.1;

}  // namespace

void run_runtime_render_phases(
    RuntimeRenderDriverContext& context,
    double& title_cooldown,
    RuntimeStorySaveExistsCache* story_save_cache) {
    if (title_cooldown >= kWindowTitleUpdateSeconds) {
        title_cooldown = 0.0;
        update_window_title(
            context.window,
            context.simulation,
            context.options,
            context.options_menu_state,
            context.main_menu_state,
            context.menu_state,
            context.story_menu_state,
            context.story_intro_state,
            context.story_runtime,
            context.story_hub_state,
            context.app_state,
            context.main_menu_row_name_fn,
            context.options_menu_row_name_fn,
            context.quick_row_name_fn,
            context.story_menu_row_name_fn,
            context.story_intro_phase_name_fn,
            context.story_hub_row_name_fn,
            context.mode_name_fn,
            context.style_name_fn,
            context.story_match_kind_name_fn);
    }

    MatchExitPolicy pause_exit_policy {};
    const MatchExitPolicy* pause_exit_policy_ptr = nullptr;
    if (context.app_state == AppState::Paused) {
        pause_exit_policy = compute_runtime_match_exit_policy(
            context.simulation,
            context.app_state,
            context.pause_return_state,
            context.match_flow,
            context.story_runtime,
            context.story_intro_state);
        pause_exit_policy_ptr = &pause_exit_policy;
    }

    render_runtime_frame(
        context.window,
        context.simulation,
        context.match_flow,
        context.show_dev_info,
        context.ai_controls_player_paddle,
        context.app_state,
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
        context.options,
        context.controls,
        context.audio_settings,
        pause_exit_policy_ptr,
        story_save_cache);
    render_visual_transition_overlay(context.window, context.visual_transition);
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
