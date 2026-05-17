#include "story_continue_resume.hpp"

#include "story_script_catalog.hpp"
#include "story_runtime_invariants.hpp"
#include "story_runtime.hpp"

namespace whacker::app {

StoryOnboardingStep normalize_onboarding_resume_step(const StoryOnboardingStep step) {
    if (step == StoryOnboardingStep::None ||
        step == StoryOnboardingStep::AyaFriendlyMatch ||
        step == StoryOnboardingStep::Complete) {
        return StoryOnboardingStep::EarlyArrivalScene;
    }
    if (step == StoryOnboardingStep::EntryBenchmarkMatch) {
        return StoryOnboardingStep::ClubIntroScene;
    }
    if (step == StoryOnboardingStep::Imagination1967Match) {
        return StoryOnboardingStep::AtHomeYoutubeScene;
    }
    if (step == StoryOnboardingStep::TixMidweekScene) {
        return StoryOnboardingStep::TixMidweekScene;
    }
    if (step == StoryOnboardingStep::PostTixLunchScene) {
        return StoryOnboardingStep::PostTixLunchScene;
    }
    return step;
}

AppState apply_continue_loaded_career(
    StoryRuntimeState& story_runtime,
    const StoryCareerData& loaded_career) {
    story_runtime.career = loaded_career;
    (void)story_graph_initialize_career_node(story_runtime.career);
    story_runtime.career_loaded = true;
    story_runtime.active_match = StoryMatchKind::None;
    story_runtime.active_rival_id = StoryRivalId::None;
    story_runtime.active_rival_style = AiStyle::Balanced;
    story_runtime.active_rival_skills = {};
    story_runtime.official_games_left = 0;
    story_runtime.official_games_right = 0;
    copy_onboarding_career_to_runtime(story_runtime);
    story_runtime.post_forfeit_scene_pending = false;

    if (!story_runtime.career.joined_club) {
        queue_story_onboarding_scene(
            story_runtime,
            normalize_onboarding_resume_step(story_runtime.onboarding_step));
        return AppState::StoryScene;
    }

    if (story_runtime.onboarding_step == StoryOnboardingStep::PostTixLunchScene) {
        queue_story_onboarding_scene(story_runtime, StoryOnboardingStep::PostTixLunchScene);
        return AppState::StoryScene;
    }

    const bool tix_midweek_pending =
        story_runtime.career.tix_1967_seen &&
        !story_runtime.career.tix_midweek_scene_seen &&
        !story_runtime.career.tix_lunch_match_declined &&
        !story_runtime.career.tix_lunch_match_completed;
    if (tix_midweek_pending) {
        story_runtime.onboarding_step = StoryOnboardingStep::TixMidweekScene;
        clear_story_runtime_scene_pending_flags(story_runtime);
        return AppState::StoryHub;
    }

    story_runtime.onboarding_step = StoryOnboardingStep::Complete;
    clear_story_runtime_scene_pending_flags(story_runtime);
    return AppState::StoryHub;
}

}  // namespace whacker::app
