#include "story_onboarding_flow.hpp"

#include "story_script_catalog.hpp"
#include "story_runtime_invariants.hpp"

namespace whacker::app {

void route_after_completed_story_match(
    StoryRuntimeState& story_runtime,
    const StoryMatchKind completed_kind,
    AppState& app_state) {
    const StoryMatchPolicyDescriptor& policy = story_match_policy_for_kind(completed_kind);
    switch (policy.post_route_completed) {
        case StoryMatchPostRoute::OnboardingClubIntroScene:
            queue_story_onboarding_scene(story_runtime, StoryOnboardingStep::ClubIntroScene);
            app_state = AppState::StoryScene;
            return;
        case StoryMatchPostRoute::OnboardingCoachBriefScene:
            queue_story_onboarding_scene(story_runtime, StoryOnboardingStep::CoachBriefScene);
            app_state = AppState::StoryScene;
            return;
        case StoryMatchPostRoute::StoryScene:
            app_state = AppState::StoryScene;
            return;
        case StoryMatchPostRoute::StoryHub:
        default:
            clear_story_runtime_scene_pending_flags(story_runtime);
            app_state = AppState::StoryHub;
            return;
    }
}

}  // namespace whacker::app
