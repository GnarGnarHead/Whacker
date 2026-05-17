#include <cassert>

#include "story_runtime_invariants.hpp"

namespace {

void test_story_runtime_invariants_accept_valid_playing_story_match() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.active_match = whacker::app::StoryMatchKind::Official;

    const auto violations = whacker::app::evaluate_story_runtime_invariants(
        whacker::app::AppState::Playing,
        runtime);
    static_cast<void>(violations);
    assert(violations == whacker::app::StoryRuntimeInvariantViolation::None);
    assert(whacker::app::story_runtime_invariants_hold(whacker::app::AppState::Playing, runtime));
}

void test_story_runtime_invariants_accept_valid_paused_story_match() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.active_match = whacker::app::StoryMatchKind::Training;

    const auto violations = whacker::app::evaluate_story_runtime_invariants(
        whacker::app::AppState::Paused,
        runtime);
    static_cast<void>(violations);
    assert(violations == whacker::app::StoryRuntimeInvariantViolation::None);
    assert(whacker::app::story_runtime_invariants_hold(whacker::app::AppState::Paused, runtime));
}

void test_story_runtime_invariants_detect_active_match_outside_play_or_pause() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.active_match = whacker::app::StoryMatchKind::Official;

    const auto violations = whacker::app::evaluate_story_runtime_invariants(
        whacker::app::AppState::StoryHub,
        runtime);
    static_cast<void>(violations);
    assert(whacker::app::has_story_runtime_invariant_violation(
        violations,
        whacker::app::StoryRuntimeInvariantViolation::ActiveMatchOutsidePlayingOrPaused));
    assert(!whacker::app::story_runtime_invariants_hold(whacker::app::AppState::StoryHub, runtime));
}

void test_story_runtime_invariants_detect_onboarding_pending_without_step() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_scene_pending = true;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::None;

    const auto violations = whacker::app::evaluate_story_runtime_invariants(
        whacker::app::AppState::StoryScene,
        runtime);
    static_cast<void>(violations);
    assert(whacker::app::has_story_runtime_invariant_violation(
        violations,
        whacker::app::StoryRuntimeInvariantViolation::OnboardingScenePendingWithoutStep));
    assert(!whacker::app::story_runtime_invariants_hold(whacker::app::AppState::StoryScene, runtime));
}

void test_story_runtime_invariants_detect_conflicting_scene_pending_flags() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_scene_pending = true;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::ClubIntroScene;
    runtime.post_forfeit_scene_pending = true;

    const auto violations = whacker::app::evaluate_story_runtime_invariants(
        whacker::app::AppState::StoryScene,
        runtime);
    static_cast<void>(violations);
    assert(whacker::app::has_story_runtime_invariant_violation(
        violations,
        whacker::app::StoryRuntimeInvariantViolation::ConflictingScenePendingFlags));
    assert(!whacker::app::story_runtime_invariants_hold(whacker::app::AppState::StoryScene, runtime));
}

void test_story_runtime_invariants_detect_story_hub_pending_flags() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_scene_pending = true;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;
    runtime.post_forfeit_scene_pending = true;

    const auto violations = whacker::app::evaluate_story_runtime_invariants(
        whacker::app::AppState::StoryHub,
        runtime);
    static_cast<void>(violations);
    assert(whacker::app::has_story_runtime_invariant_violation(
        violations,
        whacker::app::StoryRuntimeInvariantViolation::StoryHubWithOnboardingScenePending));
    assert(whacker::app::has_story_runtime_invariant_violation(
        violations,
        whacker::app::StoryRuntimeInvariantViolation::StoryHubWithPostForfeitPending));
    assert(!whacker::app::story_runtime_invariants_hold(whacker::app::AppState::StoryHub, runtime));
}

void test_queue_story_onboarding_scene_sets_exclusive_pending_state() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_scene_pending = false;
    runtime.post_forfeit_scene_pending = true;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::None;

    whacker::app::queue_story_onboarding_scene(
        runtime,
        whacker::app::StoryOnboardingStep::CoachBriefScene);

    assert(runtime.onboarding_scene_pending);
    assert(!runtime.post_forfeit_scene_pending);
    assert(runtime.onboarding_step == whacker::app::StoryOnboardingStep::CoachBriefScene);
}

void test_queue_story_post_forfeit_scene_sets_exclusive_pending_state() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_scene_pending = true;
    runtime.post_forfeit_scene_pending = false;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::ClubIntroScene;

    whacker::app::queue_story_post_forfeit_scene(runtime);

    assert(!runtime.onboarding_scene_pending);
    assert(runtime.post_forfeit_scene_pending);
    assert(runtime.onboarding_step == whacker::app::StoryOnboardingStep::ClubIntroScene);
}

void test_clear_story_runtime_scene_pending_flags_clears_both_flags() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_scene_pending = true;
    runtime.post_forfeit_scene_pending = true;

    whacker::app::clear_story_runtime_scene_pending_flags(runtime);

    assert(!runtime.onboarding_scene_pending);
    assert(!runtime.post_forfeit_scene_pending);
}

}  // namespace

int main() {
    test_story_runtime_invariants_accept_valid_playing_story_match();
    test_story_runtime_invariants_accept_valid_paused_story_match();
    test_story_runtime_invariants_detect_active_match_outside_play_or_pause();
    test_story_runtime_invariants_detect_onboarding_pending_without_step();
    test_story_runtime_invariants_detect_conflicting_scene_pending_flags();
    test_story_runtime_invariants_detect_story_hub_pending_flags();
    test_queue_story_onboarding_scene_sets_exclusive_pending_state();
    test_queue_story_post_forfeit_scene_sets_exclusive_pending_state();
    test_clear_story_runtime_scene_pending_flags_clears_both_flags();
    return 0;
}
