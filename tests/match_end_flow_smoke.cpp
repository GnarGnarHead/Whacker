#include <cassert>
#include <string>

#include "match_end_flow.hpp"

namespace whacker::app {

void finalize_story_match(
    StoryRuntimeState& story_runtime,
    StoryHubState& /*story_hub_state*/,
    const whacker::sim::RallyState& /*final_state*/,
    const StorySaveCareerFn /*save_career_fn*/,
    const StoryMatchEndReason /*end_reason*/) {
    story_runtime.active_match = StoryMatchKind::None;
}

void route_after_completed_story_match(
    StoryRuntimeState& story_runtime,
    const StoryMatchKind completed_kind,
    AppState& app_state) {
    if (completed_kind == StoryMatchKind::OnboardingAyaFriendly) {
        story_runtime.onboarding_step = StoryOnboardingStep::ClubIntroScene;
        story_runtime.onboarding_scene_pending = true;
        story_runtime.post_forfeit_scene_pending = false;
        app_state = AppState::StoryScene;
        return;
    }
    if (completed_kind == StoryMatchKind::OnboardingEntry) {
        story_runtime.onboarding_step = StoryOnboardingStep::CoachBriefScene;
        story_runtime.onboarding_scene_pending = true;
        story_runtime.post_forfeit_scene_pending = false;
        app_state = AppState::StoryScene;
        return;
    }
    story_runtime.onboarding_scene_pending = false;
    story_runtime.post_forfeit_scene_pending = false;
    app_state = AppState::StoryHub;
}

}  // namespace whacker::app

namespace whacker::app::story_text {

std::string forfeit_recorded_line() {
    return "Forfeit recorded.";
}

}  // namespace whacker::app::story_text

namespace {

int g_save_call_count = 0;
whacker::app::StoryCareerData g_saved_career {};

void reset_save_capture() {
    g_save_call_count = 0;
    g_saved_career = whacker::app::StoryCareerData {};
}

bool capture_save(const whacker::app::StoryCareerData& career, std::string* save_error) {
    ++g_save_call_count;
    g_saved_career = career;
    if (save_error != nullptr) {
        save_error->clear();
    }
    return true;
}

bool capture_save_failure(const whacker::app::StoryCareerData& career, std::string* save_error) {
    ++g_save_call_count;
    g_saved_career = career;
    if (save_error != nullptr) {
        *save_error = "Disk full";
    }
    return false;
}

void assert_onboarding_runtime_copied_to_career(const whacker::app::StoryRuntimeState& story_runtime) {
    static_cast<void>(story_runtime);
    assert(story_runtime.career.onboarding_step == story_runtime.onboarding_step);
    assert(story_runtime.career.onboarding_style_hint == story_runtime.onboarding_style_hint);
    assert(story_runtime.career.onboarding_performance_hint == story_runtime.onboarding_performance_hint);
    assert(story_runtime.career.onboarding_aya_feedback_available == story_runtime.onboarding_aya_feedback_available);
    assert(story_runtime.career.onboarding_aya_feedback_from_loss == story_runtime.onboarding_aya_feedback_from_loss);
    assert(story_runtime.career.onboarding_aya_feedback_hint == story_runtime.onboarding_aya_feedback_hint);
    assert(story_runtime.career.onboarding_aya_forfeited == story_runtime.onboarding_aya_forfeited);
}

void assert_saved_matches_runtime_onboarding(const whacker::app::StoryRuntimeState& story_runtime) {
    static_cast<void>(story_runtime);
    assert(g_saved_career.onboarding_step == story_runtime.onboarding_step);
    assert(g_saved_career.onboarding_style_hint == story_runtime.onboarding_style_hint);
    assert(g_saved_career.onboarding_performance_hint == story_runtime.onboarding_performance_hint);
    assert(g_saved_career.onboarding_aya_feedback_available == story_runtime.onboarding_aya_feedback_available);
    assert(g_saved_career.onboarding_aya_feedback_from_loss == story_runtime.onboarding_aya_feedback_from_loss);
    assert(g_saved_career.onboarding_aya_feedback_hint == story_runtime.onboarding_aya_feedback_hint);
    assert(g_saved_career.onboarding_aya_forfeited == story_runtime.onboarding_aya_forfeited);
}

void test_quick_match_completion_routes_to_setup() {
    reset_save_capture();
    whacker::app::StoryRuntimeState story_runtime {};
    whacker::app::StoryHubState story_hub {};
    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::Quick;
    whacker::sim::Simulation simulation {};
    whacker::app::StorySceneState story_scene {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;

    whacker::app::end_active_or_quick_match(
        story_runtime,
        story_hub,
        match_flow,
        simulation,
        story_scene,
        authored_transition_request,
        app_state,
        whacker::app::StoryMatchEndReason::Completed,
        3,
        capture_save);

    assert(app_state == whacker::app::AppState::QuickMatchSetup);
    assert(match_flow.mode == whacker::app::ActiveMatchMode::None);
    assert(g_save_call_count == 0);
}

void test_official_forfeit_routes_to_support_scene() {
    reset_save_capture();
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::Official;
    story_runtime.career.prefers_right_side = false;
    story_runtime.onboarding_scene_pending = true;
    story_runtime.onboarding_step = whacker::app::StoryOnboardingStep::ClubIntroScene;
    story_runtime.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Technical;
    story_runtime.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::CloseLoss;
    story_runtime.onboarding_aya_feedback_available = true;
    story_runtime.onboarding_aya_feedback_from_loss = true;
    story_runtime.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Spin;
    story_runtime.onboarding_aya_forfeited = true;
    whacker::app::StoryHubState story_hub {};
    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::StoryOfficial;
    whacker::sim::Simulation simulation {};
    whacker::app::StorySceneState story_scene {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;

    whacker::app::end_active_or_quick_match(
        story_runtime,
        story_hub,
        match_flow,
        simulation,
        story_scene,
        authored_transition_request,
        app_state,
        whacker::app::StoryMatchEndReason::Forfeit,
        3,
        capture_save);

    assert(app_state == whacker::app::AppState::StoryScene);
    assert(story_runtime.post_forfeit_scene_pending);
    assert(!story_runtime.onboarding_scene_pending);
    assert(story_runtime.active_match == whacker::app::StoryMatchKind::None);
    assert(match_flow.mode == whacker::app::ActiveMatchMode::None);
    assert_onboarding_runtime_copied_to_career(story_runtime);
    assert(g_save_call_count == 1);
    assert_saved_matches_runtime_onboarding(story_runtime);
}

void test_onboarding_entry_forfeit_routes_to_retry_scene() {
    reset_save_capture();
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::OnboardingEntry;
    story_runtime.career.prefers_right_side = false;
    story_runtime.post_forfeit_scene_pending = true;
    story_runtime.onboarding_step = whacker::app::StoryOnboardingStep::EntryBenchmarkMatch;
    story_runtime.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Power;
    story_runtime.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::Neutral;
    story_runtime.onboarding_aya_feedback_available = false;
    story_runtime.onboarding_aya_feedback_from_loss = false;
    story_runtime.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Balanced;
    story_runtime.onboarding_aya_forfeited = false;
    whacker::app::StoryHubState story_hub {};
    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::StoryTraining;
    whacker::sim::Simulation simulation {};
    whacker::app::StorySceneState story_scene {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;

    whacker::app::end_active_or_quick_match(
        story_runtime,
        story_hub,
        match_flow,
        simulation,
        story_scene,
        authored_transition_request,
        app_state,
        whacker::app::StoryMatchEndReason::Forfeit,
        3,
        capture_save);

    assert(app_state == whacker::app::AppState::StoryScene);
    assert(story_runtime.onboarding_scene_pending);
    assert(!story_runtime.post_forfeit_scene_pending);
    assert(story_runtime.onboarding_step == whacker::app::StoryOnboardingStep::EntryRetryScene);
    assert(story_runtime.active_match == whacker::app::StoryMatchKind::None);
    assert_onboarding_runtime_copied_to_career(story_runtime);
    assert(g_save_call_count == 1);
    assert_saved_matches_runtime_onboarding(story_runtime);
}

void test_onboarding_entry_completion_routes_to_coach_brief_scene() {
    reset_save_capture();
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::OnboardingEntry;
    story_runtime.career.prefers_right_side = false;
    story_runtime.onboarding_step = whacker::app::StoryOnboardingStep::EntryBenchmarkMatch;
    whacker::app::StoryHubState story_hub {};
    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::StoryTraining;
    whacker::sim::Simulation simulation {};
    whacker::app::StorySceneState story_scene {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;

    whacker::app::end_active_or_quick_match(
        story_runtime,
        story_hub,
        match_flow,
        simulation,
        story_scene,
        authored_transition_request,
        app_state,
        whacker::app::StoryMatchEndReason::Completed,
        3,
        capture_save);

    assert(app_state == whacker::app::AppState::StoryScene);
    assert(story_runtime.onboarding_scene_pending);
    assert(story_runtime.onboarding_step == whacker::app::StoryOnboardingStep::CoachBriefScene);
    assert(story_runtime.active_match == whacker::app::StoryMatchKind::None);
    assert_onboarding_runtime_copied_to_career(story_runtime);
    assert(g_save_call_count == 1);
    assert_saved_matches_runtime_onboarding(story_runtime);
}

void test_training_stop_routes_to_hub_and_saves_once() {
    reset_save_capture();
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::Training;
    story_runtime.career.prefers_right_side = true;
    story_runtime.onboarding_scene_pending = true;
    story_runtime.post_forfeit_scene_pending = true;
    story_runtime.onboarding_step = whacker::app::StoryOnboardingStep::ClubIntroScene;
    story_runtime.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Spin;
    story_runtime.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::BigWin;
    story_runtime.onboarding_aya_feedback_available = true;
    story_runtime.onboarding_aya_feedback_from_loss = false;
    story_runtime.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Technical;
    story_runtime.onboarding_aya_forfeited = false;
    whacker::app::StoryHubState story_hub {};
    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::StoryTraining;
    whacker::sim::Simulation simulation {};
    whacker::app::StorySceneState story_scene {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;

    whacker::app::end_active_or_quick_match(
        story_runtime,
        story_hub,
        match_flow,
        simulation,
        story_scene,
        authored_transition_request,
        app_state,
        whacker::app::StoryMatchEndReason::EndTraining,
        3,
        capture_save);

    assert(app_state == whacker::app::AppState::StoryHub);
    assert(story_runtime.active_match == whacker::app::StoryMatchKind::None);
    assert(!story_runtime.onboarding_scene_pending);
    assert(!story_runtime.post_forfeit_scene_pending);
    assert(match_flow.mode == whacker::app::ActiveMatchMode::None);
    assert_onboarding_runtime_copied_to_career(story_runtime);
    assert(g_save_call_count == 1);
    assert_saved_matches_runtime_onboarding(story_runtime);
}

void test_save_failure_surfaces_feedback() {
    reset_save_capture();
    whacker::app::StoryRuntimeState story_runtime {};
    story_runtime.active_match = whacker::app::StoryMatchKind::Training;
    story_runtime.career.prefers_right_side = false;
    whacker::app::StoryHubState story_hub {};
    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::StoryTraining;
    whacker::sim::Simulation simulation {};
    whacker::app::StorySceneState story_scene {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::AppState app_state = whacker::app::AppState::Playing;

    whacker::app::end_active_or_quick_match(
        story_runtime,
        story_hub,
        match_flow,
        simulation,
        story_scene,
        authored_transition_request,
        app_state,
        whacker::app::StoryMatchEndReason::EndTraining,
        3,
        capture_save_failure);

    assert(app_state == whacker::app::AppState::StoryHub);
    assert(g_save_call_count == 1);
    assert(story_hub.feedback_line_1 == "Save failed. Progress not written.");
    assert(story_hub.feedback_line_2 == "Disk full");
}

}  // namespace

int main() {
    test_quick_match_completion_routes_to_setup();
    test_official_forfeit_routes_to_support_scene();
    test_onboarding_entry_forfeit_routes_to_retry_scene();
    test_onboarding_entry_completion_routes_to_coach_brief_scene();
    test_training_stop_routes_to_hub_and_saves_once();
    test_save_failure_surfaces_feedback();
    return 0;
}
