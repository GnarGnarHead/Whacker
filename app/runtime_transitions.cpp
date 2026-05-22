#include "runtime_transitions.hpp"

#include "match_end_flow.hpp"
#include "story_runtime_invariants.hpp"
#include "story_runtime.hpp"
#include "text_utils.hpp"

namespace whacker::app {

MatchExitPolicy compute_runtime_match_exit_policy(
    const whacker::sim::Simulation& simulation,
    const AppState app_state,
    const AppState pause_return_state,
    const MatchFlowState& match_flow,
    const StoryRuntimeState& story_runtime,
    const StoryIntroState& story_intro_state) {
    const auto& state = simulation.state();
    const MatchProgress progress = make_match_progress(
        app_state,
        pause_return_state,
        match_flow,
        story_runtime,
        static_cast<int>(story_intro_state.phase),
        story_intro_state.points_played,
        state.left_score,
        state.right_score);
    return evaluate_match_exit_policy(progress);
}

void execute_runtime_pause_exit(
    const MatchExitPolicy& policy,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    StorySceneState& story_scene_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    AppState& app_state,
    const int story_official_games_to_win,
    const StorySanitizeNameFn sanitize_name_fn,
    const StorySaveCareerCallback save_career_fn) {
    (void)sanitize_name_fn;

    switch (policy.action) {
        case MatchExitAction::ExitQuickToSetup:
            end_active_or_quick_match(
                story_runtime,
                story_hub_state,
                match_flow,
                simulation,
                story_scene_state,
                authored_transition_request,
                app_state,
                StoryMatchEndReason::Completed,
                story_official_games_to_win,
                save_career_fn);
            return;
        case MatchExitAction::ExitStoryMatch:
            end_active_or_quick_match(
                story_runtime,
                story_hub_state,
                match_flow,
                simulation,
                story_scene_state,
                authored_transition_request,
                app_state,
                policy.story_end_reason,
                story_official_games_to_win,
                save_career_fn);
            return;
        case MatchExitAction::ExitIntroContinueStory: {
            const whacker::sim::RallyState terminal_state = simulation.state();
            story_intro_state.player_won = false;
            story_intro_state.player_forfeited = true;
            story_intro_state.final_left_score = terminal_state.left_score;
            story_intro_state.final_right_score = terminal_state.right_score;
            if (trim_copy(story_intro_state.entered_name).empty()) {
                story_intro_state.entered_name = "PLAYER";
            }
            story_intro_state.phase = StoryIntroPhase::RivalIntro;
            story_intro_state.phase_timer = 0.0f;
            reset_story_intro_typewriter(story_intro_state);
            reset_match_flow(match_flow);
            simulation.reset();
            clear_authored_transition_request(authored_transition_request);
            app_state = AppState::StoryIntro;
            return;
        }
        case MatchExitAction::None:
        default:
            return;
    }
}

void quit_runtime_to_main_menu(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    StorySceneState& story_scene_state,
    MatchFlowState& match_flow,
    PauseMenuState& pause_menu_state,
    AppState& pause_return_state,
    whacker::sim::Simulation& simulation,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    const int story_official_games_to_win,
    const StorySaveCareerCallback save_career_fn,
    AppState& app_state) {
    if (story_runtime.active_match == StoryMatchKind::Training) {
        end_active_or_quick_match(
            story_runtime,
            story_hub_state,
            match_flow,
            simulation,
            story_scene_state,
            authored_transition_request,
            app_state,
            StoryMatchEndReason::EndTraining,
            story_official_games_to_win,
            save_career_fn);
    }
    story_intro_state = StoryIntroState {};
    clear_story_scene(story_scene_state);
    clear_story_runtime_scene_pending_flags(story_runtime);
    clear_authored_transition_request(authored_transition_request);
    story_runtime.active_match = StoryMatchKind::None;
    story_runtime.official_games_left = 0;
    story_runtime.official_games_right = 0;
    reset_story_match_tracking(story_runtime);
    reset_match_flow(match_flow);
    simulation.reset();
    pause_return_state = AppState::Playing;
    pause_menu_state.selected_row = PauseMenuRowResume;
    pause_menu_state.confirm_forfeit = false;
    pause_menu_state.confirm_selected = 0;
    app_state = AppState::MainMenu;
}

}  // namespace whacker::app
