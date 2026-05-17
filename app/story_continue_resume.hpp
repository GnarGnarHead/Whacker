#pragma once

#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

StoryOnboardingStep normalize_onboarding_resume_step(StoryOnboardingStep step);

AppState apply_continue_loaded_career(
    StoryRuntimeState& story_runtime,
    const StoryCareerData& loaded_career);

}  // namespace whacker::app
