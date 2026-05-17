#include "runtime_pause.hpp"

#ifdef WHACKER_HAS_GLFW

#include "runtime_transitions.hpp"

namespace whacker::app {

PauseInputFeedback handle_runtime_pause_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    const ControlBindings& controls,
    const MatchExitPolicy& exit_policy,
    PauseMenuState& pause_menu_state,
    AppState& app_state,
    AppState& pause_return_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    StorySceneState& story_scene_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    const int story_official_games_to_win,
    const StorySanitizeNameFn sanitize_name_fn,
    const StorySaveCareerCallback save_career_fn) {
    PauseInputFeedback feedback {};

    if ((!exit_policy.has_exit_option || !exit_policy.can_exit_now || !exit_policy.requires_confirmation) &&
        pause_menu_state.confirm_forfeit) {
        pause_menu_state.confirm_forfeit = false;
        pause_menu_state.confirm_selected = 0;
    }

    if (pause_menu_state.confirm_forfeit) {
        bool toggle_choice = false;
        toggle_choice = consume_key_press(window, GLFW_KEY_LEFT, edge_state.left) || toggle_choice;
        toggle_choice = consume_key_press(window, GLFW_KEY_RIGHT, edge_state.right) || toggle_choice;
        toggle_choice = consume_menu_up_press(window, edge_state, controls) || toggle_choice;
        toggle_choice = consume_menu_down_press(window, edge_state, controls) || toggle_choice;
        if (toggle_choice) {
            pause_menu_state.confirm_selected = 1 - pause_menu_state.confirm_selected;
            feedback.play_menu_move = true;
        }
        if (consume_confirm_press(window, edge_state)) {
            feedback.play_menu_confirm = true;
            if (pause_menu_state.confirm_selected == 1) {
                execute_runtime_pause_exit(
                    exit_policy,
                    story_runtime,
                    story_hub_state,
                    story_intro_state,
                    match_flow,
                    simulation,
                    story_scene_state,
                    authored_transition_request,
                    app_state,
                    story_official_games_to_win,
                    sanitize_name_fn,
                    save_career_fn);
            } else {
                pause_menu_state.confirm_forfeit = false;
                pause_menu_state.confirm_selected = 0;
            }
        }
        return feedback;
    }

    const bool has_forfeit_row = exit_policy.has_exit_option;
    const int pause_row_count = has_forfeit_row ? PauseMenuRowCount : (PauseMenuRowCount - 1);
    if (pause_menu_state.selected_row >= pause_row_count) {
        pause_menu_state.selected_row = pause_row_count - 1;
    }
    if (consume_menu_up_press(window, edge_state, controls)) {
        pause_menu_state.selected_row = (pause_menu_state.selected_row + pause_row_count - 1) % pause_row_count;
        feedback.play_menu_move = true;
    }
    if (consume_menu_down_press(window, edge_state, controls)) {
        pause_menu_state.selected_row = (pause_menu_state.selected_row + 1) % pause_row_count;
        feedback.play_menu_move = true;
    }
    if (!consume_confirm_press(window, edge_state)) {
        return feedback;
    }

    feedback.play_menu_confirm = true;
    if (pause_menu_state.selected_row == PauseMenuRowResume) {
        app_state = pause_return_state;
        return feedback;
    }

    const bool selected_forfeit_row = has_forfeit_row && pause_menu_state.selected_row == PauseMenuRowExitMatch;
    if (selected_forfeit_row) {
        if (!exit_policy.can_exit_now) {
            pause_menu_state.selected_row = PauseMenuRowResume;
        } else if (!exit_policy.requires_confirmation) {
            execute_runtime_pause_exit(
                exit_policy,
                story_runtime,
                story_hub_state,
                story_intro_state,
                match_flow,
                simulation,
                story_scene_state,
                authored_transition_request,
                app_state,
                story_official_games_to_win,
                sanitize_name_fn,
                save_career_fn);
        } else {
            pause_menu_state.confirm_forfeit = true;
            pause_menu_state.confirm_selected = 0;
        }
        return feedback;
    }

    quit_runtime_to_main_menu(
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        match_flow,
        pause_menu_state,
        pause_return_state,
        simulation,
        authored_transition_request,
        story_official_games_to_win,
        save_career_fn,
        app_state);
    return feedback;
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
