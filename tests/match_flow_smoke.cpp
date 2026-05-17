#include <cassert>

#include "match_exit_policy.hpp"
#include "match_flow.hpp"

namespace {

whacker::sim::RallyState score_state(const int left, const int right) {
    whacker::sim::RallyState state {};
    state.left_score = left;
    state.right_score = right;
    return state;
}

void test_table_tennis_game_complete() {
    int winner = 0;
    static_cast<void>(winner);
    assert(!whacker::app::table_tennis_game_complete(10, 10, &winner));
    assert(winner == 0);
    assert(!whacker::app::table_tennis_game_complete(11, 10, &winner));
    assert(winner == 0);
    assert(whacker::app::table_tennis_game_complete(12, 10, &winner));
    assert(winner == 1);
    assert(whacker::app::table_tennis_game_complete(13, 15, &winner));
    assert(winner == -1);
}

void test_serve_block_helpers() {
    assert(!whacker::app::table_tennis_deuce_serve_mode(9, 10, true));
    assert(whacker::app::table_tennis_deuce_serve_mode(10, 10, true));
    assert(!whacker::app::table_tennis_deuce_serve_mode(10, 10, false));

    assert(whacker::app::table_tennis_serves_before_switch(9, 9, true) == 2);
    assert(whacker::app::table_tennis_serves_before_switch(10, 10, true) == 1);
    assert(whacker::app::table_tennis_serves_before_switch(10, 10, false) == 2);
}

void test_serve_rotation_regular_and_deuce() {
    whacker::sim::Simulation simulation;
    whacker::app::MatchFlowState flow {};
    whacker::app::start_match_flow(
        flow,
        whacker::app::ActiveMatchMode::Quick,
        true,
        true);

    assert(flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);

    whacker::app::update_serve_after_scored_point(flow, score_state(1, 0), simulation);
    assert(flow.serve_to_right);
    assert(flow.serves_by_current_server == 1);

    whacker::app::update_serve_after_scored_point(flow, score_state(1, 1), simulation);
    assert(!flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);

    whacker::app::update_serve_after_scored_point(flow, score_state(2, 1), simulation);
    assert(!flow.serve_to_right);
    assert(flow.serves_by_current_server == 1);

    whacker::app::update_serve_after_scored_point(flow, score_state(2, 2), simulation);
    assert(flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);

    // Force "second serve in block" then cross into deuce: server should switch exactly once.
    flow.serve_to_right = true;
    flow.serves_by_current_server = 1;
    whacker::app::update_serve_after_scored_point(flow, score_state(10, 10), simulation);
    assert(!flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);

    // In deuce mode, serve switches every point.
    whacker::app::update_serve_after_scored_point(flow, score_state(11, 10), simulation);
    assert(flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);

    whacker::app::update_serve_after_scored_point(flow, score_state(11, 11), simulation);
    assert(!flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);
}

void test_serve_rotation_without_deuce_mode() {
    whacker::sim::Simulation simulation;
    whacker::app::MatchFlowState flow {};
    whacker::app::start_match_flow(
        flow,
        whacker::app::ActiveMatchMode::Quick,
        true,
        false);

    whacker::app::update_serve_after_scored_point(flow, score_state(10, 10), simulation);
    assert(flow.serve_to_right);
    assert(flow.serves_by_current_server == 1);
    whacker::app::update_serve_after_scored_point(flow, score_state(11, 10), simulation);
    assert(!flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);
}

void test_next_game_opening_server_alternates() {
    whacker::sim::Simulation simulation;
    whacker::app::MatchFlowState flow {};
    whacker::app::start_match_flow(
        flow,
        whacker::app::ActiveMatchMode::StoryOfficial,
        true,
        true);

    assert(flow.opening_serve_to_right);
    assert(flow.serve_to_right);

    whacker::app::start_next_table_tennis_game(flow, simulation, true);
    assert(!flow.opening_serve_to_right);
    assert(!flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);

    whacker::app::start_next_table_tennis_game(flow, simulation, true);
    assert(flow.opening_serve_to_right);
    assert(flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);

    whacker::app::start_next_table_tennis_game(flow, simulation, false);
    assert(flow.opening_serve_to_right);
    assert(flow.serve_to_right);
    assert(flow.serves_by_current_server == 0);
}

void test_match_exit_policy_intro_lock() {
    whacker::app::MatchFlowState flow {};
    flow.mode = whacker::app::ActiveMatchMode::StoryTraining;
    whacker::app::StoryRuntimeState runtime {};
    const whacker::app::MatchProgress locked = whacker::app::make_match_progress(
        whacker::app::AppState::StoryIntro,
        whacker::app::AppState::StoryIntro,
        flow,
        runtime,
        1,
        3,
        2,
        1);
    const whacker::app::MatchExitPolicy locked_policy = whacker::app::evaluate_match_exit_policy(locked);
    static_cast<void>(locked_policy);
    assert(!locked_policy.has_exit_option);
    assert(!locked_policy.can_exit_now);
    assert(!locked_policy.requires_confirmation);
    assert(locked_policy.action == whacker::app::MatchExitAction::None);
    assert(!locked_policy.blocked_reason.empty());

    const whacker::app::MatchProgress unlocked = whacker::app::make_match_progress(
        whacker::app::AppState::StoryIntro,
        whacker::app::AppState::StoryIntro,
        flow,
        runtime,
        1,
        4,
        2,
        2);
    const whacker::app::MatchExitPolicy unlocked_policy = whacker::app::evaluate_match_exit_policy(unlocked);
    static_cast<void>(unlocked_policy);
    assert(unlocked_policy.has_exit_option);
    assert(unlocked_policy.can_exit_now);
    assert(unlocked_policy.requires_confirmation);
    assert(unlocked_policy.action == whacker::app::MatchExitAction::ExitIntroContinueStory);
}

void test_match_exit_policy_story_modes() {
    whacker::app::MatchFlowState flow {};
    whacker::app::StoryRuntimeState runtime {};

    runtime.active_match = whacker::app::StoryMatchKind::Training;
    whacker::app::MatchProgress training_progress = whacker::app::make_match_progress(
        whacker::app::AppState::Playing,
        whacker::app::AppState::Playing,
        flow,
        runtime,
        0,
        0,
        2,
        1);
    whacker::app::MatchExitPolicy training_policy = whacker::app::evaluate_match_exit_policy(training_progress);
    static_cast<void>(training_policy);
    assert(training_policy.has_exit_option);
    assert(training_policy.can_exit_now);
    assert(!training_policy.requires_confirmation);
    assert(training_policy.story_end_reason == whacker::app::StoryMatchEndReason::EndTraining);

    runtime.active_match = whacker::app::StoryMatchKind::Official;
    whacker::app::MatchProgress official_progress = whacker::app::make_match_progress(
        whacker::app::AppState::Playing,
        whacker::app::AppState::Playing,
        flow,
        runtime,
        0,
        0,
        4,
        5);
    whacker::app::MatchExitPolicy official_policy = whacker::app::evaluate_match_exit_policy(official_progress);
    static_cast<void>(official_policy);
    assert(official_policy.has_exit_option);
    assert(official_policy.can_exit_now);
    assert(official_policy.requires_confirmation);
    assert(official_policy.story_end_reason == whacker::app::StoryMatchEndReason::Forfeit);

    runtime.active_match = whacker::app::StoryMatchKind::OnboardingAyaFriendly;
    const whacker::app::MatchProgress aya_progress = whacker::app::make_match_progress(
        whacker::app::AppState::Playing,
        whacker::app::AppState::Playing,
        flow,
        runtime,
        0,
        0,
        3,
        4);
    const whacker::app::MatchExitPolicy aya_policy = whacker::app::evaluate_match_exit_policy(aya_progress);
    static_cast<void>(aya_policy);
    assert(aya_policy.has_exit_option);
    assert(aya_policy.can_exit_now);
    assert(aya_policy.requires_confirmation);
    assert(aya_policy.story_end_reason == whacker::app::StoryMatchEndReason::Forfeit);

    runtime.active_match = whacker::app::StoryMatchKind::OnboardingEntry;
    const whacker::app::MatchProgress benji_progress = whacker::app::make_match_progress(
        whacker::app::AppState::Playing,
        whacker::app::AppState::Playing,
        flow,
        runtime,
        0,
        0,
        3,
        4);
    const whacker::app::MatchExitPolicy benji_policy = whacker::app::evaluate_match_exit_policy(benji_progress);
    static_cast<void>(benji_policy);
    assert(!benji_policy.has_exit_option);
    assert(!benji_policy.can_exit_now);
    assert(!benji_policy.requires_confirmation);
    assert(benji_policy.action == whacker::app::MatchExitAction::None);
}

void test_match_exit_policy_quick_mode_from_pause() {
    whacker::app::MatchFlowState flow {};
    flow.mode = whacker::app::ActiveMatchMode::Quick;
    whacker::app::StoryRuntimeState runtime {};
    runtime.active_match = whacker::app::StoryMatchKind::None;

    const whacker::app::MatchProgress progress = whacker::app::make_match_progress(
        whacker::app::AppState::Paused,
        whacker::app::AppState::Playing,
        flow,
        runtime,
        0,
        0,
        1,
        1);
    const whacker::app::MatchExitPolicy policy = whacker::app::evaluate_match_exit_policy(progress);
    static_cast<void>(policy);
    assert(policy.has_exit_option);
    assert(policy.can_exit_now);
    assert(!policy.requires_confirmation);
    assert(policy.action == whacker::app::MatchExitAction::ExitQuickToSetup);
}

void test_match_exit_policy_intro_lock_from_pause() {
    whacker::app::MatchFlowState flow {};
    flow.mode = whacker::app::ActiveMatchMode::StoryTraining;
    whacker::app::StoryRuntimeState runtime {};

    const whacker::app::MatchProgress locked = whacker::app::make_match_progress(
        whacker::app::AppState::Paused,
        whacker::app::AppState::StoryIntro,
        flow,
        runtime,
        2,
        3,
        2,
        1);
    const whacker::app::MatchExitPolicy locked_policy = whacker::app::evaluate_match_exit_policy(locked);
    static_cast<void>(locked_policy);
    assert(!locked_policy.has_exit_option);
    assert(!locked_policy.can_exit_now);

    const whacker::app::MatchProgress unlocked = whacker::app::make_match_progress(
        whacker::app::AppState::Paused,
        whacker::app::AppState::StoryIntro,
        flow,
        runtime,
        2,
        4,
        2,
        2);
    const whacker::app::MatchExitPolicy unlocked_policy = whacker::app::evaluate_match_exit_policy(unlocked);
    static_cast<void>(unlocked_policy);
    assert(unlocked_policy.has_exit_option);
    assert(unlocked_policy.can_exit_now);
    assert(unlocked_policy.action == whacker::app::MatchExitAction::ExitIntroContinueStory);
}

void test_match_exit_policy_intro_uses_score_sum() {
    whacker::app::MatchFlowState flow {};
    flow.mode = whacker::app::ActiveMatchMode::StoryTraining;
    whacker::app::StoryRuntimeState runtime {};

    const whacker::app::MatchProgress locked = whacker::app::make_match_progress(
        whacker::app::AppState::StoryIntro,
        whacker::app::AppState::StoryIntro,
        flow,
        runtime,
        1,
        999,
        1,
        1);
    const whacker::app::MatchExitPolicy locked_policy = whacker::app::evaluate_match_exit_policy(locked);
    static_cast<void>(locked_policy);
    assert(!locked_policy.has_exit_option);
    assert(!locked_policy.can_exit_now);

    const whacker::app::MatchProgress unlocked = whacker::app::make_match_progress(
        whacker::app::AppState::StoryIntro,
        whacker::app::AppState::StoryIntro,
        flow,
        runtime,
        1,
        0,
        2,
        2);
    const whacker::app::MatchExitPolicy unlocked_policy = whacker::app::evaluate_match_exit_policy(unlocked);
    static_cast<void>(unlocked_policy);
    assert(unlocked_policy.has_exit_option);
    assert(unlocked_policy.can_exit_now);
}

}  // namespace

int main() {
    test_table_tennis_game_complete();
    test_serve_block_helpers();
    test_serve_rotation_regular_and_deuce();
    test_serve_rotation_without_deuce_mode();
    test_next_game_opening_server_alternates();
    test_match_exit_policy_intro_lock();
    test_match_exit_policy_story_modes();
    test_match_exit_policy_quick_mode_from_pause();
    test_match_exit_policy_intro_lock_from_pause();
    test_match_exit_policy_intro_uses_score_sum();
    return 0;
}
