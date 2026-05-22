#include <cassert>

#include "story_classification.hpp"
#include "story_onboarding_flow.hpp"
#include "story_runtime.hpp"
#include "story_script_catalog.hpp"

namespace {

void test_completed_match_routing() {
    whacker::app::StoryRuntimeState runtime {};

    runtime.post_forfeit_scene_pending = true;
    whacker::app::Screen route = whacker::app::route_after_completed_story_match(
        runtime,
        whacker::app::StoryMatchKind::OnboardingAyaFriendly);
    assert(route == whacker::app::Screen::StoryScene);
    assert(runtime.onboarding_step == whacker::app::StoryOnboardingStep::ClubIntroScene);
    assert(runtime.onboarding_scene_pending);
    assert(!runtime.post_forfeit_scene_pending);

    runtime.onboarding_scene_pending = false;
    runtime.post_forfeit_scene_pending = true;
    route = whacker::app::route_after_completed_story_match(
        runtime,
        whacker::app::StoryMatchKind::OnboardingEntry);
    assert(route == whacker::app::Screen::StoryScene);
    assert(runtime.onboarding_step == whacker::app::StoryOnboardingStep::CoachBriefScene);
    assert(runtime.onboarding_scene_pending);
    assert(!runtime.post_forfeit_scene_pending);

    runtime.onboarding_scene_pending = true;
    runtime.post_forfeit_scene_pending = true;
    route = whacker::app::route_after_completed_story_match(
        runtime,
        whacker::app::StoryMatchKind::Official);
    assert(route == whacker::app::Screen::StoryHub);
    assert(!runtime.onboarding_scene_pending);
    assert(!runtime.post_forfeit_scene_pending);
}

void test_authored_wipe_directives() {
    assert(
        whacker::app::story_onboarding_transition_triggers_wipe(
            whacker::app::StoryOnboardingStep::CoachBriefScene,
            whacker::app::StoryOnboardingStep::AtHomeYoutubeScene));
    assert(
        !whacker::app::story_onboarding_transition_triggers_wipe(
            whacker::app::StoryOnboardingStep::EarlyArrivalScene,
            whacker::app::StoryOnboardingStep::AyaFriendlyMatch));

    const whacker::app::StoryMatchPolicyDescriptor& onboarding_entry =
        whacker::app::story_match_policy_for_kind(whacker::app::StoryMatchKind::OnboardingEntry);
    assert(!whacker::app::story_policy_post_route_triggers_wipe(onboarding_entry, false));
    assert(!whacker::app::story_policy_post_route_triggers_wipe(onboarding_entry, true));
}

void test_style_classification() {
    whacker::progression::SkillUsageAccumulator balanced {};
    balanced.contacts = 10;
    balanced.sum_abs_u = 4.0f;
    balanced.sum_power_samples = 4.2f;
    balanced.sum_spin_inject_samples = 4.1f;
    static_cast<void>(balanced);
    assert(
        whacker::app::classify_story_style_hint(balanced) ==
        whacker::app::StoryIntroStyleHint::Balanced);

    whacker::progression::SkillUsageAccumulator power {};
    power.contacts = 10;
    power.sum_abs_u = 1.8f;
    power.sum_power_samples = 8.5f;
    power.sum_spin_inject_samples = 1.2f;
    static_cast<void>(power);
    assert(
        whacker::app::classify_story_style_hint(power) ==
        whacker::app::StoryIntroStyleHint::Power);
}

void test_weakness_classification() {
    whacker::progression::SkillUsageMetrics usage {};
    usage.power = 0.9f;
    usage.edge = 0.4f;
    usage.spin_inject = 0.2f;
    static_cast<void>(usage);
    assert(
        whacker::app::classify_story_weakness_hint(usage) ==
        whacker::app::StoryIntroStyleHint::Spin);

    usage.power = 0.15f;
    usage.edge = 0.55f;
    usage.spin_inject = 0.45f;
    assert(
        whacker::app::classify_story_weakness_hint(usage) ==
        whacker::app::StoryIntroStyleHint::Power);
}

void test_performance_classification() {
    assert(
        whacker::app::classify_story_performance_hint(true, 11, 7) ==
        whacker::app::StoryIntroPerformanceHint::BigWin);
    assert(
        whacker::app::classify_story_performance_hint(false, 10, 12) ==
        whacker::app::StoryIntroPerformanceHint::CloseLoss);
    assert(
        whacker::app::classify_story_performance_hint(false, 6, 11) ==
        whacker::app::StoryIntroPerformanceHint::Neutral);
}

void test_onboarding_sync_runtime_to_career_round_trip() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;
    runtime.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Technical;
    runtime.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::CloseLoss;
    runtime.onboarding_aya_feedback_available = true;
    runtime.onboarding_aya_feedback_from_loss = true;
    runtime.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Spin;
    runtime.onboarding_aya_forfeited = true;

    whacker::app::copy_onboarding_runtime_to_career(runtime);
    assert(runtime.career.onboarding_step == whacker::app::StoryOnboardingStep::CoachBriefScene);
    assert(runtime.career.onboarding_style_hint == whacker::app::StoryIntroStyleHint::Technical);
    assert(runtime.career.onboarding_performance_hint == whacker::app::StoryIntroPerformanceHint::CloseLoss);
    assert(runtime.career.onboarding_aya_feedback_available);
    assert(runtime.career.onboarding_aya_feedback_from_loss);
    assert(runtime.career.onboarding_aya_feedback_hint == whacker::app::StoryIntroStyleHint::Spin);
    assert(runtime.career.onboarding_aya_forfeited);

    runtime.onboarding_step = whacker::app::StoryOnboardingStep::None;
    runtime.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Balanced;
    runtime.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::Neutral;
    runtime.onboarding_aya_feedback_available = false;
    runtime.onboarding_aya_feedback_from_loss = false;
    runtime.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Balanced;
    runtime.onboarding_aya_forfeited = false;

    whacker::app::copy_onboarding_career_to_runtime(runtime);
    assert(runtime.onboarding_step == whacker::app::StoryOnboardingStep::CoachBriefScene);
    assert(runtime.onboarding_style_hint == whacker::app::StoryIntroStyleHint::Technical);
    assert(runtime.onboarding_performance_hint == whacker::app::StoryIntroPerformanceHint::CloseLoss);
    assert(runtime.onboarding_aya_feedback_available);
    assert(runtime.onboarding_aya_feedback_from_loss);
    assert(runtime.onboarding_aya_feedback_hint == whacker::app::StoryIntroStyleHint::Spin);
    assert(runtime.onboarding_aya_forfeited);
}

}  // namespace

int main() {
    test_completed_match_routing();
    test_authored_wipe_directives();
    test_style_classification();
    test_weakness_classification();
    test_performance_classification();
    test_onboarding_sync_runtime_to_career_round_trip();
    return 0;
}
