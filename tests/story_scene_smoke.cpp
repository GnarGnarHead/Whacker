#include <cassert>
#include <string>

#include "story_scene.hpp"
#include "story_text.hpp"

namespace {

void test_post_forfeit_scene_preempts_onboarding_step() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.post_forfeit_scene_pending = true;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::EarlyArrivalScene;
    runtime.career.official_forfeit_streak = 2;
    whacker::app::StorySceneState scene {};

    whacker::app::begin_story_onboarding_scene(scene, runtime);

    assert(scene.id == whacker::app::StorySceneId::PostForfeitSupport);
    assert(scene.header == whacker::app::story_text::post_forfeit_scene_header());
    assert(scene.line_count == 3);
    assert(scene.lines[0] == whacker::app::story_text::post_forfeit_scene_line_1(2));
    assert(scene.lines[1] == whacker::app::story_text::post_forfeit_scene_line_2(2));
    assert(scene.lines[2] == whacker::app::story_text::post_forfeit_scene_line_3(2));
    assert(scene.portrait_ids[0] == whacker::app::StoryPortraitId::CoachReyes);
    assert(scene.portrait_ids[1] == whacker::app::StoryPortraitId::Aya);
    assert(scene.portrait_ids[2] == whacker::app::StoryPortraitId::Tix);
    assert(scene.dialogue_writing);
    assert(whacker::app::story_scene_has_content(scene));
}

void test_early_arrival_scene_content() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::EarlyArrivalScene;
    runtime.career.player_name = "SCOTT";
    runtime.career.prefers_right_side = true;
    whacker::app::StorySceneState scene {};

    whacker::app::begin_story_onboarding_scene(scene, runtime);

    assert(scene.id == whacker::app::StorySceneId::OnboardingEarlyArrival);
    assert(scene.header == whacker::app::story_text::onboarding_early_arrival_header());
    assert(scene.line_count == 4);
    assert(scene.lines[0] == whacker::app::story_text::onboarding_aya_early_arrival_line_1());
    assert(scene.lines[1] == whacker::app::story_text::onboarding_aya_early_arrival_line_2("SCOTT"));
    assert(scene.lines[2] == whacker::app::story_text::onboarding_aya_early_arrival_line_3());
    assert(scene.lines[3] == whacker::app::story_text::onboarding_aya_early_arrival_line_4());
    assert(scene.player_is_right);
    assert(scene.portrait_ids[0] == whacker::app::StoryPortraitId::Aya);
    assert(scene.portrait_ids[1] == whacker::app::StoryPortraitId::Aya);
    assert(scene.portrait_ids[2] == whacker::app::StoryPortraitId::Aya);
    assert(scene.portrait_ids[3] == whacker::app::StoryPortraitId::Aya);
    assert(scene.dialogue_writing);
}

void test_club_intro_scene_with_aya_feedback() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::ClubIntroScene;
    runtime.career.player_name = "SCOTT";
    runtime.onboarding_aya_feedback_available = true;
    runtime.onboarding_aya_forfeited = true;
    whacker::app::StorySceneState scene {};

    whacker::app::begin_story_onboarding_scene(scene, runtime);

    assert(scene.id == whacker::app::StorySceneId::OnboardingClubIntro);
    assert(scene.header == whacker::app::story_text::onboarding_club_floor_header());
    assert(scene.line_count == 6);
    assert(scene.lines[0] == whacker::app::story_text::onboarding_aya_forfeit_feedback_line());
    assert(scene.lines[1] == whacker::app::story_text::onboarding_aya_intro_to_coach_line("SCOTT"));
    assert(scene.lines[2] == whacker::app::story_text::onboarding_coach_intro_player_line("SCOTT"));
    assert(scene.lines[3] == whacker::app::story_text::onboarding_coach_welcome_line());
    assert(scene.lines[4] == whacker::app::story_text::onboarding_coach_assign_benji_line());
    assert(scene.lines[5] == whacker::app::story_text::onboarding_coach_benji_spin_warning_line());
    assert(scene.portrait_ids[0] == whacker::app::StoryPortraitId::Aya);
    assert(scene.portrait_ids[1] == whacker::app::StoryPortraitId::Aya);
    assert(scene.portrait_ids[2] == whacker::app::StoryPortraitId::CoachReyes);
    assert(scene.portrait_ids[5] == whacker::app::StoryPortraitId::CoachReyes);
}

void test_coach_brief_and_entry_retry_scene_content() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;
    runtime.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::CloseLoss;
    runtime.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Technical;
    whacker::app::StorySceneState scene {};

    whacker::app::begin_story_onboarding_scene(scene, runtime);
    assert(scene.id == whacker::app::StorySceneId::OnboardingCoachBrief);
    assert(scene.header == whacker::app::story_text::onboarding_coach_reyes_header());
    assert(scene.line_count == 9);
    assert(
        scene.lines[0] ==
        whacker::app::story_text::onboarding_coach_post_entry_compliment_line(
            whacker::app::StoryIntroPerformanceHint::CloseLoss,
            whacker::app::StoryIntroStyleHint::Technical));
    assert(scene.lines[8] == whacker::app::story_text::onboarding_tix_post_day_line_5());
    assert(scene.portrait_ids[0] == whacker::app::StoryPortraitId::CoachReyes);
    assert(scene.portrait_ids[4] == whacker::app::StoryPortraitId::Tix);
    assert(scene.portrait_ids[8] == whacker::app::StoryPortraitId::Tix);

    runtime.onboarding_step = whacker::app::StoryOnboardingStep::EntryRetryScene;
    whacker::app::begin_story_onboarding_scene(scene, runtime);
    assert(scene.id == whacker::app::StorySceneId::OnboardingEntryRetry);
    assert(scene.header == whacker::app::story_text::onboarding_coach_reyes_header());
    assert(scene.line_count == 1);
    assert(scene.lines[0] == whacker::app::story_text::onboarding_coach_entry_retry_line());
}

void test_at_home_youtube_scene_content_minimal_prelude() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::AtHomeYoutubeScene;
    whacker::app::StorySceneState scene {};

    whacker::app::begin_story_onboarding_scene(scene, runtime);

    assert(scene.id == whacker::app::StorySceneId::PostBenjiAtHomeYoutube);
    assert(scene.header == whacker::app::story_text::at_home_youtube_header());
    assert(scene.line_count == 2);
    assert(scene.lines[0] == whacker::app::story_text::at_home_youtube_line_1());
    assert(scene.lines[1] == whacker::app::story_text::at_home_youtube_line_2());
    assert(scene.lines[0].rfind("SEARCH:", 0) == 0);
    assert(scene.lines[1].rfind("IMAGINE:", 0) == 0);
    assert(scene.portrait_ids[0] == whacker::app::StoryPortraitId::None);
    assert(scene.portrait_ids[1] == whacker::app::StoryPortraitId::None);
    for (int i = 0; i < scene.line_count; ++i) {
        assert(scene.speakers[static_cast<std::size_t>(i)] != whacker::app::StorySceneSpeaker::Player);
    }
}

void test_invalid_step_clears_scene() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::None;
    whacker::app::StorySceneState scene {};
    scene.id = whacker::app::StorySceneId::OnboardingClubIntro;
    scene.line_count = 2;
    scene.lines[0] = "stale";
    scene.dialogue_writing = true;

    whacker::app::begin_story_onboarding_scene(scene, runtime);

    assert(scene.id == whacker::app::StorySceneId::None);
    assert(scene.line_count == 0);
    assert(!scene.dialogue_writing);
    assert(!whacker::app::story_scene_has_content(scene));
}

void test_tix_midweek_scene_exposes_binary_lunch_choice() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::TixMidweekScene;
    runtime.career.player_name = "SCOTT";
    whacker::app::StorySceneState scene {};

    whacker::app::begin_story_onboarding_scene(scene, runtime);

    assert(scene.id == whacker::app::StorySceneId::TixMidweekLunchInvite);
    assert(scene.header == whacker::app::story_text::tix_midweek_scene_header());
    assert(scene.line_count == 5);
    assert(scene.lines[0] == whacker::app::story_text::tix_midweek_scene_line_1());
    assert(scene.lines[1] == whacker::app::story_text::tix_midweek_scene_line_2());
    assert(scene.lines[2] == whacker::app::story_text::tix_midweek_scene_line_3());
    assert(scene.lines[3] == whacker::app::story_text::tix_midweek_scene_line_4());
    assert(scene.lines[4] == whacker::app::story_text::tix_midweek_scene_line_5());
    assert(scene.portrait_ids[0] == whacker::app::StoryPortraitId::Tix);
    assert(scene.portrait_ids[4] == whacker::app::StoryPortraitId::Tix);
    assert(scene.has_binary_choice);
    assert(scene.binary_choice_yes_selected);
    assert(scene.dialogue_writing);
}

void test_scene_scroll_resets_to_latest_on_typewriter_controls() {
    whacker::app::StorySceneState scene {};
    scene.id = whacker::app::StorySceneId::OnboardingClubIntro;
    scene.line_count = 1;
    scene.line_index = 0;
    scene.lines[0] = "AYA: KEEP IT CLEAN.";
    scene.dialogue_writing = true;
    scene.scroll_lines_from_bottom = 3;

    whacker::app::reset_story_scene_typewriter(scene);
    assert(scene.scroll_lines_from_bottom == 0);

    scene.scroll_lines_from_bottom = 2;
    scene.visible_chars = 0;
    scene.dialogue_writing = true;
    whacker::app::update_story_scene_typewriter(scene, 0.2f, 1.0f);
    assert(scene.scroll_lines_from_bottom == 0);

    scene.scroll_lines_from_bottom = 4;
    whacker::app::reveal_story_scene_current_line(scene);
    assert(scene.scroll_lines_from_bottom == 0);
}

}  // namespace

int main() {
    test_post_forfeit_scene_preempts_onboarding_step();
    test_early_arrival_scene_content();
    test_club_intro_scene_with_aya_feedback();
    test_coach_brief_and_entry_retry_scene_content();
    test_at_home_youtube_scene_content_minimal_prelude();
    test_invalid_step_clears_scene();
    test_tix_midweek_scene_exposes_binary_lunch_choice();
    test_scene_scroll_resets_to_latest_on_typewriter_controls();
    return 0;
}
