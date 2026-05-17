#include <cassert>
#include <string>

#include "runtime_transitions.hpp"

namespace {

struct EndCallCapture {
    bool called = false;
    whacker::app::StoryMatchEndReason reason = whacker::app::StoryMatchEndReason::Completed;
};

EndCallCapture g_end_call {};
bool g_end_call_received_save_callback = false;
bool g_complete_story_intro_called = false;
whacker::app::StoryIntroState g_complete_story_intro_capture {};
bool g_complete_story_intro_received_save_callback = false;
bool g_reset_story_tracking_called = false;

bool fake_save_career(const whacker::app::StoryCareerData&, std::string* save_error) {
    if (save_error != nullptr) {
        save_error->clear();
    }
    return true;
}

void reset_captures() {
    g_end_call = EndCallCapture {};
    g_end_call_received_save_callback = false;
    g_complete_story_intro_called = false;
    g_complete_story_intro_capture = whacker::app::StoryIntroState {};
    g_complete_story_intro_received_save_callback = false;
    g_reset_story_tracking_called = false;
}

void test_compute_runtime_match_exit_policy_intro_gate() {
    whacker::sim::Simulation simulation {};
    auto& sim_state = simulation.mutable_state();
    sim_state.left_score = 2;
    sim_state.right_score = 1;

    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::StoryTraining;
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryIntroState intro {};
    intro.phase = whacker::app::StoryIntroPhase::PlayMatch;

    const whacker::app::MatchExitPolicy locked_policy =
        whacker::app::compute_runtime_match_exit_policy(
            simulation,
            whacker::app::AppState::StoryIntro,
            whacker::app::AppState::StoryIntro,
            match_flow,
            runtime,
            intro);
    static_cast<void>(locked_policy);
    assert(!locked_policy.has_exit_option);
    assert(!locked_policy.can_exit_now);
    assert(locked_policy.action == whacker::app::MatchExitAction::None);

    sim_state.right_score = 2;  // 4 total balls played
    const whacker::app::MatchExitPolicy unlocked_policy =
        whacker::app::compute_runtime_match_exit_policy(
            simulation,
            whacker::app::AppState::StoryIntro,
            whacker::app::AppState::StoryIntro,
            match_flow,
            runtime,
            intro);
    static_cast<void>(unlocked_policy);
    assert(unlocked_policy.has_exit_option);
    assert(unlocked_policy.can_exit_now);
    assert(unlocked_policy.requires_confirmation);
    assert(unlocked_policy.action == whacker::app::MatchExitAction::ExitIntroContinueStory);
}

void test_quit_runtime_to_main_menu_resets_state() {
    whacker::sim::Simulation simulation {};
    simulation.mutable_state().left_score = 4;
    simulation.mutable_state().right_score = 5;

    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    runtime.onboarding_scene_pending = true;
    runtime.post_forfeit_scene_pending = true;
    runtime.active_match = whacker::app::StoryMatchKind::Official;
    runtime.official_games_left = 2;
    runtime.official_games_right = 1;

    whacker::app::StoryIntroState intro {};
    intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    intro.entered_name = "SCOTT";

    whacker::app::StorySceneState scene {};
    scene.id = whacker::app::StorySceneId::OnboardingClubIntro;
    scene.line_count = 1;

    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::StoryOfficial;
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};

    whacker::app::PauseMenuState pause_menu {};
    pause_menu.selected_row = whacker::app::PauseMenuRowExitMatch;
    pause_menu.confirm_forfeit = true;
    pause_menu.confirm_selected = 1;

    whacker::app::AppState pause_return_state = whacker::app::AppState::StoryIntro;
    whacker::app::AppState app_state = whacker::app::AppState::Playing;

    reset_captures();
    whacker::app::quit_runtime_to_main_menu(
        runtime,
        hub,
        intro,
        scene,
        match_flow,
        pause_menu,
        pause_return_state,
        simulation,
        authored_transition_request,
        3,
        fake_save_career,
        app_state);

    assert(!g_end_call.called);
    assert(app_state == whacker::app::AppState::MainMenu);
    assert(pause_return_state == whacker::app::AppState::Playing);
    assert(!runtime.onboarding_scene_pending);
    assert(!runtime.post_forfeit_scene_pending);
    assert(runtime.active_match == whacker::app::StoryMatchKind::None);
    assert(runtime.official_games_left == 0);
    assert(runtime.official_games_right == 0);
    assert(g_reset_story_tracking_called);
    assert(match_flow.mode == whacker::app::ActiveMatchMode::None);
    assert(intro.phase == whacker::app::StoryIntroPhase::Invite);
    assert(!pause_menu.confirm_forfeit);
    assert(pause_menu.confirm_selected == 0);
    assert(pause_menu.selected_row == whacker::app::PauseMenuRowResume);
    assert(scene.id == whacker::app::StorySceneId::None);
    assert(simulation.state().left_score == 0);
    assert(simulation.state().right_score == 0);
}

void test_quit_runtime_to_main_menu_training_uses_end_training_reason() {
    whacker::sim::Simulation simulation {};
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    runtime.active_match = whacker::app::StoryMatchKind::Training;
    whacker::app::StoryIntroState intro {};
    whacker::app::StorySceneState scene {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::PauseMenuState pause_menu {};
    whacker::app::AppState pause_return_state = whacker::app::AppState::Playing;
    whacker::app::AppState app_state = whacker::app::AppState::Playing;

    reset_captures();
    whacker::app::quit_runtime_to_main_menu(
        runtime,
        hub,
        intro,
        scene,
        match_flow,
        pause_menu,
        pause_return_state,
        simulation,
        authored_transition_request,
        3,
        fake_save_career,
        app_state);

    assert(g_end_call.called);
    assert(g_end_call.reason == whacker::app::StoryMatchEndReason::EndTraining);
    assert(g_end_call_received_save_callback);
    assert(app_state == whacker::app::AppState::MainMenu);
}

void test_execute_runtime_pause_exit_routes_to_match_end_flow() {
    whacker::sim::Simulation simulation {};
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    whacker::app::StoryIntroState intro {};
    whacker::app::StorySceneState scene {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::AppState app_state = whacker::app::AppState::Paused;

    reset_captures();
    whacker::app::MatchExitPolicy policy_story {};
    policy_story.action = whacker::app::MatchExitAction::ExitStoryMatch;
    policy_story.story_end_reason = whacker::app::StoryMatchEndReason::Forfeit;
    whacker::app::execute_runtime_pause_exit(
        policy_story,
        runtime,
        hub,
        intro,
        match_flow,
        simulation,
        scene,
        authored_transition_request,
        app_state,
        3,
        nullptr,
        fake_save_career);
    assert(g_end_call.called);
    assert(g_end_call.reason == whacker::app::StoryMatchEndReason::Forfeit);
    assert(g_end_call_received_save_callback);

    reset_captures();
    whacker::app::MatchExitPolicy policy_quick {};
    policy_quick.action = whacker::app::MatchExitAction::ExitQuickToSetup;
    whacker::app::execute_runtime_pause_exit(
        policy_quick,
        runtime,
        hub,
        intro,
        match_flow,
        simulation,
        scene,
        authored_transition_request,
        app_state,
        3,
        nullptr,
        fake_save_career);
    assert(g_end_call.called);
    assert(g_end_call.reason == whacker::app::StoryMatchEndReason::Completed);
    assert(g_end_call_received_save_callback);
}

void test_execute_runtime_pause_exit_intro_continue_uses_player_defaults() {
    whacker::sim::Simulation simulation {};
    simulation.mutable_state().left_score = 4;
    simulation.mutable_state().right_score = 5;

    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    whacker::app::StoryIntroState intro {};
    intro.entered_name = "   ";
    whacker::app::StorySceneState scene {};

    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::AppState app_state = whacker::app::AppState::Paused;

    reset_captures();
    whacker::app::MatchExitPolicy policy {};
    policy.action = whacker::app::MatchExitAction::ExitIntroContinueStory;
    whacker::app::execute_runtime_pause_exit(
        policy,
        runtime,
        hub,
        intro,
        match_flow,
        simulation,
        scene,
        authored_transition_request,
        app_state,
        3,
        nullptr,
        fake_save_career);
    assert(!g_complete_story_intro_called);
    assert(!g_complete_story_intro_received_save_callback);
    assert(!intro.player_won);
    assert(intro.player_forfeited);
    assert(intro.final_left_score == 4);
    assert(intro.final_right_score == 5);
    assert(intro.entered_name == "PLAYER");
    assert(intro.phase == whacker::app::StoryIntroPhase::RivalIntro);
    assert(app_state == whacker::app::AppState::StoryIntro);
    assert(simulation.state().left_score == 0);
    assert(simulation.state().right_score == 0);
}

}  // namespace

namespace whacker::app {

void end_active_or_quick_match(
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    StorySceneState& /*story_scene_state*/,
    RuntimeAuthoredTransitionRequest& /*authored_transition_request*/,
    AppState& /*app_state*/,
    const StoryMatchEndReason end_reason,
    int /*story_official_games_to_win*/,
    StorySaveCareerCallback save_career_fn) {
    g_end_call.called = true;
    g_end_call.reason = end_reason;
    g_end_call_received_save_callback = save_career_fn != nullptr;
}

void complete_story_intro(
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    StoryIntroState& story_intro_state,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    AppState& /*app_state*/,
    RuntimeAuthoredTransitionRequest& /*authored_transition_request*/,
    StorySanitizeNameFn /*sanitize_name_fn*/,
    StorySaveCareerCallback save_career_fn) {
    g_complete_story_intro_called = true;
    g_complete_story_intro_capture = story_intro_state;
    g_complete_story_intro_received_save_callback = save_career_fn != nullptr;
}

void reset_story_intro_typewriter(StoryIntroState& story_intro_state) {
    story_intro_state.visible_chars = 0;
    story_intro_state.type_accum = 0.0f;
    story_intro_state.typed_phase = story_intro_state.phase;
    story_intro_state.typed_break = story_intro_state.break_kind;
    story_intro_state.dialogue_writing = true;
}

void clear_story_scene(StorySceneState& scene_state) {
    scene_state = StorySceneState {};
}

void reset_story_match_tracking(StoryRuntimeState& story_runtime) {
    story_runtime.player_usage = {};
    story_runtime.active_match_seconds = 0.0f;
    story_runtime.active_peak_lead = 0;
    story_runtime.active_peak_deficit = 0;
    g_reset_story_tracking_called = true;
}

}  // namespace whacker::app

int main() {
    test_compute_runtime_match_exit_policy_intro_gate();
    test_quit_runtime_to_main_menu_resets_state();
    test_quit_runtime_to_main_menu_training_uses_end_training_reason();
    test_execute_runtime_pause_exit_routes_to_match_end_flow();
    test_execute_runtime_pause_exit_intro_continue_uses_player_defaults();
    return 0;
}
