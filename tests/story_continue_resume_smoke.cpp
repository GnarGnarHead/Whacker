#include <cassert>

#include "story_continue_resume.hpp"

namespace {

void test_normalize_onboarding_resume_step() {
    using whacker::app::StoryOnboardingStep;
    using whacker::app::normalize_onboarding_resume_step;

    assert(normalize_onboarding_resume_step(StoryOnboardingStep::None) == StoryOnboardingStep::EarlyArrivalScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::AyaFriendlyMatch) ==
        StoryOnboardingStep::EarlyArrivalScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::Complete) ==
        StoryOnboardingStep::EarlyArrivalScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::EntryBenchmarkMatch) ==
        StoryOnboardingStep::ClubIntroScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::ClubIntroScene) ==
        StoryOnboardingStep::ClubIntroScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::CoachBriefScene) ==
        StoryOnboardingStep::CoachBriefScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::EntryRetryScene) ==
        StoryOnboardingStep::EntryRetryScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::AtHomeYoutubeScene) ==
        StoryOnboardingStep::AtHomeYoutubeScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::Imagination1967Match) ==
        StoryOnboardingStep::AtHomeYoutubeScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::TixMidweekScene) ==
        StoryOnboardingStep::TixMidweekScene);
    assert(
        normalize_onboarding_resume_step(StoryOnboardingStep::PostTixLunchScene) ==
        StoryOnboardingStep::PostTixLunchScene);
}

void test_apply_continue_loaded_career_routes_unjoined_to_story_scene() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.active_match = whacker::app::StoryMatchKind::Official;
    runtime.official_games_left = 2;
    runtime.official_games_right = 1;
    runtime.post_forfeit_scene_pending = true;

    whacker::app::StoryCareerData loaded {};
    loaded.joined_club = false;
    loaded.week = 4;
    loaded.onboarding_step = whacker::app::StoryOnboardingStep::EntryBenchmarkMatch;
    loaded.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Spin;
    loaded.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::CloseLoss;
    loaded.onboarding_aya_feedback_available = true;
    loaded.onboarding_aya_feedback_from_loss = true;
    loaded.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Technical;
    loaded.onboarding_aya_forfeited = true;

    const whacker::app::AppState next = whacker::app::apply_continue_loaded_career(runtime, loaded);
    static_cast<void>(next);

    assert(next == whacker::app::AppState::StoryScene);
    assert(runtime.career_loaded);
    assert(runtime.career.week == 4);
    assert(runtime.active_match == whacker::app::StoryMatchKind::None);
    assert(runtime.official_games_left == 0);
    assert(runtime.official_games_right == 0);
    assert(!runtime.post_forfeit_scene_pending);
    assert(runtime.onboarding_step == whacker::app::StoryOnboardingStep::ClubIntroScene);
    assert(runtime.onboarding_scene_pending);
    assert(runtime.onboarding_style_hint == whacker::app::StoryIntroStyleHint::Spin);
    assert(runtime.onboarding_performance_hint == whacker::app::StoryIntroPerformanceHint::CloseLoss);
    assert(runtime.onboarding_aya_feedback_available);
    assert(runtime.onboarding_aya_feedback_from_loss);
    assert(runtime.onboarding_aya_feedback_hint == whacker::app::StoryIntroStyleHint::Technical);
    assert(runtime.onboarding_aya_forfeited);
}

void test_apply_continue_loaded_career_routes_joined_to_story_hub() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.post_forfeit_scene_pending = true;

    whacker::app::StoryCareerData loaded {};
    loaded.joined_club = true;
    loaded.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;

    const whacker::app::AppState next = whacker::app::apply_continue_loaded_career(runtime, loaded);
    static_cast<void>(next);

    assert(next == whacker::app::AppState::StoryHub);
    assert(runtime.career_loaded);
    assert(runtime.onboarding_step == whacker::app::StoryOnboardingStep::Complete);
    assert(!runtime.onboarding_scene_pending);
    assert(!runtime.post_forfeit_scene_pending);
    assert(runtime.active_match == whacker::app::StoryMatchKind::None);
    assert(runtime.official_games_left == 0);
    assert(runtime.official_games_right == 0);
}

void test_apply_continue_loaded_career_keeps_tix_midweek_offer_for_joined_club() {
    whacker::app::StoryRuntimeState runtime {};

    whacker::app::StoryCareerData loaded {};
    loaded.joined_club = true;
    loaded.tix_1967_seen = true;
    loaded.onboarding_step = whacker::app::StoryOnboardingStep::Complete;
    loaded.tix_midweek_scene_seen = false;
    loaded.tix_lunch_match_declined = false;
    loaded.tix_lunch_match_completed = false;

    const whacker::app::AppState next = whacker::app::apply_continue_loaded_career(runtime, loaded);
    static_cast<void>(next);

    assert(next == whacker::app::AppState::StoryHub);
    assert(runtime.career_loaded);
    assert(runtime.onboarding_step == whacker::app::StoryOnboardingStep::TixMidweekScene);
    assert(!runtime.onboarding_scene_pending);
    assert(!runtime.post_forfeit_scene_pending);
}

void test_apply_continue_loaded_career_routes_post_tix_lunch_scene_for_joined_club() {
    whacker::app::StoryRuntimeState runtime {};

    whacker::app::StoryCareerData loaded {};
    loaded.joined_club = true;
    loaded.onboarding_step = whacker::app::StoryOnboardingStep::PostTixLunchScene;

    const whacker::app::AppState next = whacker::app::apply_continue_loaded_career(runtime, loaded);
    static_cast<void>(next);

    assert(next == whacker::app::AppState::StoryScene);
    assert(runtime.career_loaded);
    assert(runtime.onboarding_step == whacker::app::StoryOnboardingStep::PostTixLunchScene);
    assert(runtime.onboarding_scene_pending);
    assert(!runtime.post_forfeit_scene_pending);
}

}  // namespace

int main() {
    test_normalize_onboarding_resume_step();
    test_apply_continue_loaded_career_routes_unjoined_to_story_scene();
    test_apply_continue_loaded_career_routes_joined_to_story_hub();
    test_apply_continue_loaded_career_keeps_tix_midweek_offer_for_joined_club();
    test_apply_continue_loaded_career_routes_post_tix_lunch_scene_for_joined_club();
    return 0;
}
