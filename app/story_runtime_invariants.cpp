#include "story_runtime_invariants.hpp"

namespace whacker::app {

StoryRuntimeInvariantViolation evaluate_story_runtime_invariants(
    const AppState app_state,
    const StoryRuntimeState& story_runtime) {
    StoryRuntimeInvariantViolation issues = StoryRuntimeInvariantViolation::None;

    if (story_runtime.active_match != StoryMatchKind::None &&
        app_state != AppState::Playing &&
        app_state != AppState::Paused) {
        issues |= StoryRuntimeInvariantViolation::ActiveMatchOutsidePlayingOrPaused;
    }

    if (story_runtime.onboarding_scene_pending && story_runtime.onboarding_step == StoryOnboardingStep::None) {
        issues |= StoryRuntimeInvariantViolation::OnboardingScenePendingWithoutStep;
    }

    if (story_runtime.onboarding_scene_pending && story_runtime.active_match != StoryMatchKind::None) {
        issues |= StoryRuntimeInvariantViolation::OnboardingScenePendingWithActiveMatch;
    }

    if (story_runtime.onboarding_scene_pending && story_runtime.post_forfeit_scene_pending) {
        issues |= StoryRuntimeInvariantViolation::ConflictingScenePendingFlags;
    }

    if (app_state == AppState::StoryHub) {
        if (story_runtime.onboarding_scene_pending) {
            issues |= StoryRuntimeInvariantViolation::StoryHubWithOnboardingScenePending;
        }
        if (story_runtime.post_forfeit_scene_pending) {
            issues |= StoryRuntimeInvariantViolation::StoryHubWithPostForfeitPending;
        }
    }

    return issues;
}

bool story_runtime_invariants_hold(
    const AppState app_state,
    const StoryRuntimeState& story_runtime) {
    return evaluate_story_runtime_invariants(app_state, story_runtime) == StoryRuntimeInvariantViolation::None;
}

void clear_story_runtime_scene_pending_flags(StoryRuntimeState& story_runtime) {
    story_runtime.onboarding_scene_pending = false;
    story_runtime.post_forfeit_scene_pending = false;
}

void queue_story_onboarding_scene(
    StoryRuntimeState& story_runtime,
    const StoryOnboardingStep step) {
    story_runtime.onboarding_step = step;
    story_runtime.onboarding_scene_pending = step != StoryOnboardingStep::None;
    story_runtime.post_forfeit_scene_pending = false;
}

void queue_story_post_forfeit_scene(StoryRuntimeState& story_runtime) {
    story_runtime.onboarding_scene_pending = false;
    story_runtime.post_forfeit_scene_pending = true;
}

}  // namespace whacker::app
