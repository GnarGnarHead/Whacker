#include "window_title.hpp"

#ifdef WHACKER_HAS_GLFW

#include <string>

#include <GLFW/glfw3.h>

#include "paddle_tuning.hpp"

namespace whacker::app {

void update_window_title(
    GLFWwindow* window,
    const whacker::sim::Simulation& simulation,
    const MatchOptions& options,
    const OptionsMenuState& options_menu_state,
    const MainMenuState& main_menu_state,
    const MenuState& menu_state,
    const StoryMenuState& story_menu_state,
    const StoryIntroState& story_intro_state,
    const StoryRuntimeState& story_runtime,
    const StoryHubState& story_hub_state,
    const AppState app_state,
    const IntNameFn main_menu_row_name_fn,
    const IntNameFn options_menu_row_name_fn,
    const IntNameFn quick_row_name_fn,
    const IntNameFn story_menu_row_name_fn,
    const IntroPhaseNameFn story_intro_phase_name_fn,
    const IntNameFn story_hub_row_name_fn,
    const ModeNameFn mode_name_fn,
    const StyleNameFn style_name_fn,
    const MatchKindNameFn match_kind_name_fn) {
    const auto safe_int_name = [](const IntNameFn fn, const int value) -> const char* {
        return fn != nullptr ? fn(value) : "?";
    };
    const auto safe_mode_name = [](const ModeNameFn fn, const PaddleMode mode) -> const char* {
        return fn != nullptr ? fn(mode) : "?";
    };
    const auto safe_style_name = [](const StyleNameFn fn, const AiStyle style) -> const char* {
        return fn != nullptr ? fn(style) : "?";
    };
    const auto safe_kind_name = [](const MatchKindNameFn fn, const StoryMatchKind kind) -> const char* {
        return fn != nullptr ? fn(kind) : "none";
    };
    const auto safe_phase_name = [](const IntroPhaseNameFn fn, const StoryIntroPhase phase) -> const char* {
        return fn != nullptr ? fn(phase) : "?";
    };

    const auto& state = simulation.state();
    std::string title;
    if (app_state == AppState::MainMenu) {
        title = "Whacker Main Menu | row:";
        title += safe_int_name(main_menu_row_name_fn, main_menu_state.selected_row);
    } else if (app_state == AppState::OptionsMenu) {
        title = "Whacker Options | row:";
        title += safe_int_name(options_menu_row_name_fn, options_menu_state.selected_row);
    } else if (app_state == AppState::QuickMatchSetup) {
        title = "Whacker Menu | row:";
        title += safe_int_name(quick_row_name_fn, menu_state.selected_row);
        title += " | P1:";
        title += safe_mode_name(mode_name_fn, options.left_mode);
        if (options.left_mode == PaddleMode::AI) {
            title += "(";
            title += safe_style_name(style_name_fn, style_for_skills(options.left_paddle_skills));
            title += ")";
        }
        title += " | P2:";
        title += safe_mode_name(mode_name_fn, options.right_mode);
        if (options.right_mode == PaddleMode::AI) {
            title += "(";
            title += safe_style_name(style_name_fn, style_for_skills(options.right_paddle_skills));
            title += ")";
        }
    } else if (app_state == AppState::PaddleTuning) {
        title = "Whacker Paddle Tuning";
    } else if (app_state == AppState::StoryMenu) {
        title = "Whacker Story Menu | row:";
        title += safe_int_name(story_menu_row_name_fn, story_menu_state.selected_row);
    } else if (app_state == AppState::StoryIntro) {
        title = "Whacker Story Intro | phase:";
        title += safe_phase_name(story_intro_phase_name_fn, story_intro_state.phase);
    } else if (app_state == AppState::StoryScene) {
        title = "Whacker Story Scene | Club Entry";
    } else if (app_state == AppState::StoryHub) {
        title = "Whacker Story Hub | week ";
        title += std::to_string(story_runtime.career.week);
        title += " | row:";
        title += safe_int_name(story_hub_row_name_fn, story_hub_state.selected_row);
    } else if (app_state == AppState::Paused) {
        title = "Whacker Paused | L:";
        title += std::to_string(state.left_score);
        title += " R:";
        title += std::to_string(state.right_score);
        title += " | ENTER select | ESC resume";
    } else {
        title = "Whacker  L:";
        title += std::to_string(state.left_score);
        title += "  R:";
        title += std::to_string(state.right_score);
        title += "  speed:";
        title += std::to_string(state.ball.speed_scalar);
        title += "  spin:";
        title += std::to_string(state.ball.spin);
        title += "  mode:";
        title += safe_kind_name(match_kind_name_fn, story_runtime.active_match);
        if (story_runtime.active_match == StoryMatchKind::Official) {
            title += "  games:";
            title += std::to_string(story_runtime.official_games_left);
            title += "-";
            title += std::to_string(story_runtime.official_games_right);
        }
        title += "  P1:";
        title += safe_mode_name(mode_name_fn, options.left_mode);
        if (options.left_mode == PaddleMode::AI) {
            title += "(";
            title += safe_style_name(style_name_fn, style_for_skills(options.left_paddle_skills));
            title += ")";
        }
        title += "  P2:";
        title += safe_mode_name(mode_name_fn, options.right_mode);
        if (options.right_mode == PaddleMode::AI) {
            title += "(";
            title += safe_style_name(style_name_fn, style_for_skills(options.right_paddle_skills));
            title += ")";
        }
    }
    glfwSetWindowTitle(window, title.c_str());
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
