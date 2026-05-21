#include "menu_flow.hpp"

#ifdef WHACKER_HAS_GLFW

#include <string>

#include <GLFW/glfw3.h>

#include "main_menu_actions.hpp"
#include "story_match.hpp"
#include "story_runtime.hpp"
#include "story_runtime_invariants.hpp"
#include "story_save_helpers.hpp"

namespace {

int mode_index(const whacker::app::PaddleMode mode) {
    return mode == whacker::app::PaddleMode::Human ? 0 : 1;
}

whacker::app::PaddleMode mode_from_index(const int index) {
    return index == 0 ? whacker::app::PaddleMode::Human : whacker::app::PaddleMode::AI;
}

void cycle_paddle_mode(whacker::app::PaddleMode& mode, const int direction) {
    const int step = direction >= 0 ? 1 : -1;
    int index = mode_index(mode);
    index = (index + step + 2) % 2;
    mode = mode_from_index(index);
}

}  // namespace

namespace whacker::app {

void handle_main_menu_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    MainMenuState& main_menu_state,
    MenuState& quick_menu_state,
    StoryMenuState& story_menu_state,
    OptionsMenuState& options_menu_state,
    const ControlBindings& controls,
    AppState& app_state) {
    const MainMenuActionResult result = apply_main_menu_action(
        main_menu_state,
        consume_menu_up_press(window, edge_state, controls),
        consume_menu_down_press(window, edge_state, controls),
        consume_confirm_press(window, edge_state),
        false);
    if (result == MainMenuActionResult::Story) {
        story_menu_state.selected_row = StoryMenuRowContinue;
        story_menu_state.confirm_overwrite = false;
        story_menu_state.confirm_selected = 0;
        app_state = AppState::StoryMenu;
        return;
    }
    if (result == MainMenuActionResult::Quick) {
        quick_menu_state.selected_row = MenuRowP1;
        app_state = AppState::QuickMatchSetup;
        return;
    }
    if (result == MainMenuActionResult::Options) {
        options_menu_state.selected_row = OptionsMenuRowP1Up;
        options_menu_state.waiting_for_key = false;
        app_state = AppState::OptionsMenu;
        return;
    }
    if (result == MainMenuActionResult::Quit) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
}

void set_menu_row_option(MatchOptions& options, const MenuState& menu_state, const int direction) {
    if (menu_state.selected_row == MenuRowP1) {
        cycle_paddle_mode(options.left_mode, direction);
    } else if (menu_state.selected_row == MenuRowP2) {
        cycle_paddle_mode(options.right_mode, direction);
    }
}

void handle_menu_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    MenuState& menu_state,
    MatchOptions& options,
    const ControlBindings& controls,
    MatchFlowState& match_flow,
    AppState& app_state,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng) {
    if (consume_menu_up_press(window, edge_state, controls)) {
        menu_state.selected_row = (menu_state.selected_row + MenuRowCount - 1) % MenuRowCount;
    }
    if (consume_menu_down_press(window, edge_state, controls)) {
        menu_state.selected_row = (menu_state.selected_row + 1) % MenuRowCount;
    }

    int direction = 0;
    if (consume_key_press(window, GLFW_KEY_LEFT, edge_state.left)) {
        direction -= 1;
    }
    if (consume_key_press(window, GLFW_KEY_RIGHT, edge_state.right)) {
        direction += 1;
    }
    if (direction != 0) {
        set_menu_row_option(options, menu_state, direction);
    }

    if (consume_confirm_press(window, edge_state)) {
        if (menu_state.selected_row == MenuRowStart) {
            simulation.reset();
            const bool opening_serve_to_right = randomize_opening_serve(simulation, rng);
            start_match_flow(match_flow, ActiveMatchMode::Quick, opening_serve_to_right, true);
            start_match_opening_countdown(match_flow, simulation);
            app_state = AppState::Playing;
            return;
        }
        if (menu_state.selected_row == MenuRowP1Tuning || menu_state.selected_row == MenuRowP2Tuning) {
            app_state = AppState::PaddleTuning;
            return;
        }
        set_menu_row_option(options, menu_state, 1);
    }
}

void handle_story_hub_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchOptions& options,
    const ControlBindings& controls,
    MatchFlowState& match_flow,
    AppState& app_state,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    const StorySaveCareerCallback save_career_fn) {
    if (!story_runtime.career_loaded) {
        app_state = AppState::StoryMenu;
        return;
    }

    const bool tix_midweek_pending =
        story_runtime.career.joined_club &&
        story_runtime.career.tix_1967_seen &&
        !story_runtime.career.tix_midweek_scene_seen &&
        !story_runtime.career.tix_lunch_match_declined &&
        !story_runtime.career.tix_lunch_match_completed;
    if (tix_midweek_pending) {
        queue_story_onboarding_scene(story_runtime, StoryOnboardingStep::TixMidweekScene);
        copy_onboarding_runtime_to_career(story_runtime);
        (void)persist_story_career_with_feedback(story_runtime.career, save_career_fn, &story_hub_state);
        app_state = AppState::StoryScene;
        return;
    }

    if (consume_menu_up_press(window, edge_state, controls)) {
        story_hub_state.selected_row = (story_hub_state.selected_row + StoryHubRowCount - 1) % StoryHubRowCount;
    }
    if (consume_menu_down_press(window, edge_state, controls)) {
        story_hub_state.selected_row = (story_hub_state.selected_row + 1) % StoryHubRowCount;
    }

    if (!consume_confirm_press(window, edge_state)) {
        return;
    }

    const StoryHubRow row = static_cast<StoryHubRow>(story_hub_state.selected_row);
    if (!story_hub_row_enabled(row, story_runtime.career)) {
        return;
    }

    if (row == StoryHubRowBack) {
        (void)persist_story_career_with_feedback(story_runtime.career, save_career_fn, &story_hub_state);
        app_state = AppState::MainMenu;
        return;
    }

    if (row == StoryHubRowNextWeek) {
        advance_story_week(story_runtime, story_hub_state, save_career_fn);
        return;
    }

    if (row == StoryHubRowPaddleTuning) {
        app_state = AppState::PaddleTuning;
        return;
    }

    if (row == StoryHubRowOfficialMatch || row == StoryHubRowTrainingMatch) {
        const StoryMatchKind kind =
            row == StoryHubRowOfficialMatch ? StoryMatchKind::Official : StoryMatchKind::Training;
        start_story_match(story_runtime, story_hub_state, options, simulation, match_flow, rng, kind);
        app_state = AppState::Playing;
    }
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
