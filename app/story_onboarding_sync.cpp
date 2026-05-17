#include "story_runtime.hpp"

namespace whacker::app {

void copy_onboarding_runtime_to_career(StoryRuntimeState& story_runtime) {
    story_runtime.career.onboarding_step = story_runtime.onboarding_step;
    story_runtime.career.onboarding_style_hint = story_runtime.onboarding_style_hint;
    story_runtime.career.onboarding_performance_hint = story_runtime.onboarding_performance_hint;
    story_runtime.career.onboarding_aya_feedback_available = story_runtime.onboarding_aya_feedback_available;
    story_runtime.career.onboarding_aya_feedback_from_loss = story_runtime.onboarding_aya_feedback_from_loss;
    story_runtime.career.onboarding_aya_feedback_hint = story_runtime.onboarding_aya_feedback_hint;
    story_runtime.career.onboarding_aya_forfeited = story_runtime.onboarding_aya_forfeited;
}

void copy_onboarding_career_to_runtime(StoryRuntimeState& story_runtime) {
    story_runtime.onboarding_step = story_runtime.career.onboarding_step;
    story_runtime.onboarding_style_hint = story_runtime.career.onboarding_style_hint;
    story_runtime.onboarding_performance_hint = story_runtime.career.onboarding_performance_hint;
    story_runtime.onboarding_aya_feedback_available = story_runtime.career.onboarding_aya_feedback_available;
    story_runtime.onboarding_aya_feedback_from_loss = story_runtime.career.onboarding_aya_feedback_from_loss;
    story_runtime.onboarding_aya_feedback_hint = story_runtime.career.onboarding_aya_feedback_hint;
    story_runtime.onboarding_aya_forfeited = story_runtime.career.onboarding_aya_forfeited;
}

}  // namespace whacker::app
