#include "story_onboarding_flow.hpp"

#include "story_script_catalog.hpp"
#include "story_runtime_invariants.hpp"

namespace whacker::app {

Screen route_after_completed_story_match(
    StoryRuntimeState& story_runtime,
    const StoryMatchKind completed_kind) {
    const StoryMatchPolicyDescriptor& policy = story_match_policy_for_kind(completed_kind);
    switch (policy.post_route_completed) {
        case StoryMatchPostRoute::OnboardingClubIntroScene:
            queue_story_onboarding_scene(story_runtime, StoryOnboardingStep::ClubIntroScene);
            return Screen::StoryScene;
        case StoryMatchPostRoute::OnboardingCoachBriefScene:
            queue_story_onboarding_scene(story_runtime, StoryOnboardingStep::CoachBriefScene);
            return Screen::StoryScene;
        case StoryMatchPostRoute::StoryScene:
            return Screen::StoryScene;
        case StoryMatchPostRoute::StoryHub:
        default:
            clear_story_runtime_scene_pending_flags(story_runtime);
            return Screen::StoryHub;
    }
}

}  // namespace whacker::app
