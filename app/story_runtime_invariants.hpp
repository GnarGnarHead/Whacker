#pragma once

#include <cstdint>

#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

enum class StoryRuntimeInvariantViolation : std::uint32_t {
    None = 0u,
    ActiveMatchOutsidePlayingOrPaused = 1u << 0u,
    OnboardingScenePendingWithoutStep = 1u << 1u,
    OnboardingScenePendingWithActiveMatch = 1u << 2u,
    ConflictingScenePendingFlags = 1u << 3u,
    StoryHubWithOnboardingScenePending = 1u << 4u,
    StoryHubWithPostForfeitPending = 1u << 5u,
};

constexpr StoryRuntimeInvariantViolation operator|(
    const StoryRuntimeInvariantViolation lhs,
    const StoryRuntimeInvariantViolation rhs) {
    return static_cast<StoryRuntimeInvariantViolation>(
        static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

constexpr StoryRuntimeInvariantViolation& operator|=(
    StoryRuntimeInvariantViolation& lhs,
    const StoryRuntimeInvariantViolation rhs) {
    lhs = lhs | rhs;
    return lhs;
}

constexpr bool has_story_runtime_invariant_violation(
    const StoryRuntimeInvariantViolation violations,
    const StoryRuntimeInvariantViolation violation) {
    return
        (static_cast<std::uint32_t>(violations) & static_cast<std::uint32_t>(violation)) !=
        0u;
}

StoryRuntimeInvariantViolation evaluate_story_runtime_invariants(
    AppState app_state,
    const StoryRuntimeState& story_runtime);

bool story_runtime_invariants_hold(
    AppState app_state,
    const StoryRuntimeState& story_runtime);

void clear_story_runtime_scene_pending_flags(StoryRuntimeState& story_runtime);
void queue_story_onboarding_scene(StoryRuntimeState& story_runtime, StoryOnboardingStep step);
void queue_story_post_forfeit_scene(StoryRuntimeState& story_runtime);

}  // namespace whacker::app
