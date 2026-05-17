#include "runtime_render_phase.hpp"

#ifdef WHACKER_HAS_GLFW

#include "game_render.hpp"
#include "menu_overlay.hpp"
#include "options_menu_input.hpp"
#include "paddle_tuning_overlay.hpp"
#include "quick_menu_render.hpp"
#include "runtime_helpers.hpp"
#include "story_intro_overlay.hpp"
#include "story_overlays.hpp"
#include "story_save.hpp"
#include "story_scene_overlay.hpp"
#include "story_text.hpp"

namespace whacker::app {

namespace {

bool should_render_match_hud(const AppState app_state, const StoryIntroState& story_intro_state) {
    switch (app_state) {
        case AppState::Playing:
        case AppState::Paused:
            return true;
        case AppState::StoryIntro:
            return story_intro_state.phase == StoryIntroPhase::PlayMatch;
        default:
            return false;
    }
}

}  // namespace

void render_runtime_frame(
    GLFWwindow* window,
    const whacker::sim::Simulation& simulation,
    const MatchFlowState& match_flow,
    const bool show_dev_info,
    const bool ai_controls_player_paddle,
    const AppState app_state,
    const MainMenuState& main_menu_state,
    const OptionsMenuState& options_menu_state,
    const PauseMenuState& pause_menu_state,
    const MenuState& menu_state,
    const PaddleTuningState& paddle_tuning_state,
    const StoryMenuState& story_menu_state,
    const StoryIntroState& story_intro_state,
    const StorySceneState& story_scene_state,
    const StoryHubState& story_hub_state,
    const StoryRuntimeState& story_runtime,
    const MatchOptions& options,
    const ControlBindings& controls,
    const AudioSettings& audio_settings,
    const MatchExitPolicy* pause_exit_policy,
    RuntimeStorySaveExistsCache* story_save_cache) {
    const auto resolve_story_save_exists = [&]() -> bool {
        if (story_save_cache != nullptr) {
            return resolve_runtime_story_save_exists_cached(*story_save_cache, story_save_exists);
        }
        return story_save_exists();
    };

    const bool ball_visible = match_opening_ball_visible(match_flow);
    render_scene(window, simulation, ball_visible);
    if (should_render_match_hud(app_state, story_intro_state)) {
        render_hud(window, simulation);
    }
    if (app_state == AppState::Playing && story_runtime.imagination_takeover_cue_seconds > 0.0f) {
        render_play_center_message(window, story_text::imagination_takeover_cue_line());
    }
    const bool show_play_dev_overlay =
        app_state == AppState::Playing ||
        (app_state == AppState::StoryIntro && story_intro_state.phase == StoryIntroPhase::PlayMatch);
    if (show_dev_info && show_play_dev_overlay) {
        render_dev_overlay(window, simulation, ai_controls_player_paddle);
    }
    if (app_state == AppState::MainMenu) {
        render_main_menu_overlay(window, main_menu_state, main_menu_row_name);
    } else if (app_state == AppState::OptionsMenu) {
        render_options_menu_overlay(
            window,
            options_menu_state,
            controls,
            audio_settings,
            options_menu_row_name,
            key_name,
            binding_value);
    } else if (app_state == AppState::QuickMatchSetup) {
        render_menu_overlay(window, options, menu_state);
    } else if (app_state == AppState::PaddleTuning) {
        render_paddle_tuning_overlay(window, paddle_tuning_state);
    } else if (app_state == AppState::StoryMenu) {
        render_story_menu_overlay(
            window,
            story_menu_state,
            resolve_story_save_exists(),
            story_menu_row_name);
    } else if (app_state == AppState::StoryIntro) {
        render_story_intro_overlay(
            window,
            story_runtime,
            story_intro_state,
            controls,
            key_name,
            sanitize_player_name);
    } else if (app_state == AppState::StoryScene) {
        render_story_scene_overlay(window, story_scene_state);
    } else if (app_state == AppState::StoryHub) {
        render_story_hub_overlay(
            window,
            story_runtime,
            story_hub_state,
            story_hub_row_name,
            story_hub_row_enabled,
            sanitize_player_name);
    } else if (app_state == AppState::Paused) {
        if (pause_exit_policy != nullptr) {
            render_pause_overlay(window, pause_menu_state, *pause_exit_policy);
        }
    }
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
