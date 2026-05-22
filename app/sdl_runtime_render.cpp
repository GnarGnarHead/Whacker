#include "sdl_runtime_render.hpp"

#include "game_render.hpp"
#include "match_exit_policy.hpp"
#include "match_flow.hpp"
#include "menu_overlay.hpp"
#include "paddle_tuning_overlay.hpp"
#include "pixel_font.hpp"
#include "quick_menu_render.hpp"
#include "runtime_match_exit_policy.hpp"
#include "runtime_visual_transition_render.hpp"
#include "sdl_options_value_labels.hpp"
#include "sdl_runtime_labels.hpp"
#include "story_intro_overlay.hpp"
#include "story_overlays.hpp"
#include "story_runtime.hpp"
#include "story_save.hpp"
#include "story_scene_overlay.hpp"
#include "text_utils.hpp"

#include <GL/gl.h>

namespace whacker::app {

void render_runtime_frame(
    const RenderContext& render_context,
    const SdlRuntimeState& runtime,
    const whacker::sim::Simulation& simulation) {
    if (!render_context_valid(render_context)) {
        return;
    }

    glViewport(0, 0, render_context.drawable_width, render_context.drawable_height);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const ScopedPixelRenderContext scoped_pixel_context {render_context};
    const bool ball_visible =
        (runtime.app_state == AppState::Playing ||
         runtime.app_state == AppState::Paused ||
         (runtime.app_state == AppState::StoryIntro && runtime.story_intro.phase == StoryIntroPhase::PlayMatch))
            ? match_opening_ball_visible(runtime.match_flow)
            : true;
    render_scene(render_context, simulation, ball_visible);
    if (runtime.app_state == AppState::MainMenu) {
        render_main_menu_overlay(
            render_context,
            runtime.main_menu,
            main_menu_row_name,
            runtime.main_menu_feedback);
    } else if (runtime.app_state == AppState::OptionsMenu) {
        const SdlOptionsValueLabelContext value_context {
            .bindings = &runtime.input.bindings(),
            .audio_settings = &runtime.audio_settings,
        };
        render_options_menu_overlay(
            render_context,
            runtime.options_menu,
            options_menu_row_name,
            sdl_options_value_label,
            &value_context);
    } else if (runtime.app_state == AppState::QuickMatchSetup) {
        render_menu_overlay(render_context, runtime.options, runtime.quick_menu);
    } else if (runtime.app_state == AppState::PaddleTuning) {
        render_paddle_tuning_overlay(render_context, runtime.paddle_tuning);
    } else if (runtime.app_state == AppState::StoryMenu) {
        render_story_menu_overlay(
            render_context,
            runtime.story_menu,
            story_save_exists(),
            story_menu_row_name,
            runtime.story_menu_feedback);
    } else if (runtime.app_state == AppState::StoryHub) {
        render_story_hub_overlay(
            render_context,
            runtime.story_runtime,
            runtime.story_hub,
            story_hub_row_name,
            story_hub_row_enabled,
            sanitize_player_name);
    } else if (runtime.app_state == AppState::StoryIntro) {
        if (runtime.story_intro.phase == StoryIntroPhase::PlayMatch) {
            render_hud(render_context, simulation);
        }
        render_story_intro_overlay(
            render_context,
            runtime.story_runtime,
            runtime.story_intro,
            runtime.controls,
            sdl_key_name,
            sanitize_player_name);
    } else if (runtime.app_state == AppState::StoryScene) {
        render_story_scene_overlay(render_context, runtime.story_scene);
    } else {
        render_hud(render_context, simulation);
        if (runtime.app_state == AppState::Paused) {
            const MatchExitPolicy exit_policy = compute_runtime_match_exit_policy(
                simulation,
                runtime.app_state,
                runtime.pause_return_state,
                runtime.match_flow,
                runtime.story_runtime,
                runtime.story_intro);
            render_pause_overlay(render_context, runtime.pause_menu, exit_policy);
        }
    }
    render_visual_transition_overlay(render_context, runtime.visual_transition);
}

}  // namespace whacker::app
