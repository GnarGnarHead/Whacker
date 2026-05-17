#include <cstdlib>

#include "story_intro.hpp"
#include "story_script_catalog.hpp"

namespace {

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

void test_story_match_policy_catalog_defaults() {
    using namespace whacker::app;

    const StoryMatchPolicyDescriptor& training = story_match_policy_for_kind(StoryMatchKind::Training);
    require(training.match_kind == StoryMatchKind::Training);
    require(training.progression.xp_enabled);
    require(training.progression.xp_on_forfeit);
    require(training.score_model == StoryMatchScoreModel::RallyLoop);
    require(training.exit_behavior == StoryMatchExitBehavior::StopTraining);
    require(training.post_route_completed == StoryMatchPostRoute::StoryHub);
    require(!training.post_route_completed_trigger_wipe);
    require(!training.post_route_forfeit_trigger_wipe);

    const StoryMatchPolicyDescriptor& official = story_match_policy_for_kind(StoryMatchKind::Official);
    require(official.match_kind == StoryMatchKind::Official);
    require(official.progression.xp_enabled);
    require(official.progression.xp_on_forfeit);
    require(official.score_model == StoryMatchScoreModel::BestOfGames);
    require(official.exit_behavior == StoryMatchExitBehavior::Forfeit);
    require(official.post_route_forfeit == StoryMatchPostRoute::PostForfeitSupportScene);
    require(!official.post_route_completed_trigger_wipe);
    require(!official.post_route_forfeit_trigger_wipe);

    const StoryMatchPolicyDescriptor& onboarding_aya =
        story_match_policy_for_kind(StoryMatchKind::OnboardingAyaFriendly);
    require(onboarding_aya.match_kind == StoryMatchKind::OnboardingAyaFriendly);
    require(onboarding_aya.progression.xp_enabled);
    require(onboarding_aya.progression.xp_on_forfeit);
    require(onboarding_aya.score_model == StoryMatchScoreModel::SingleGame);
    require(onboarding_aya.exit_behavior == StoryMatchExitBehavior::Forfeit);
    require(onboarding_aya.post_route_completed == StoryMatchPostRoute::OnboardingClubIntroScene);
    require(!onboarding_aya.post_route_completed_trigger_wipe);
    require(!onboarding_aya.post_route_forfeit_trigger_wipe);

    const StoryMatchPolicyDescriptor& onboarding_entry =
        story_match_policy_for_kind(StoryMatchKind::OnboardingEntry);
    require(onboarding_entry.match_kind == StoryMatchKind::OnboardingEntry);
    require(onboarding_entry.progression.xp_enabled);
    require(onboarding_entry.progression.xp_on_forfeit);
    require(onboarding_entry.score_model == StoryMatchScoreModel::SingleGame);
    require(onboarding_entry.exit_behavior == StoryMatchExitBehavior::None);
    require(onboarding_entry.post_route_forfeit == StoryMatchPostRoute::OnboardingEntryRetryScene);
    require(!onboarding_entry.post_route_completed_trigger_wipe);
    require(!onboarding_entry.post_route_forfeit_trigger_wipe);

    const StoryMatchPolicyDescriptor& imagination =
        story_match_policy_for_kind(StoryMatchKind::Imagination1967);
    require(imagination.match_kind == StoryMatchKind::Imagination1967);
    require(imagination.scenario_id == StoryMatchScenarioId::Imagination1967);
    require(imagination.score_model == StoryMatchScoreModel::SingleGame);
    require(imagination.ai_preview_points == 4);
    require(imagination.exit_behavior == StoryMatchExitBehavior::None);
    require(imagination.post_route_completed == StoryMatchPostRoute::StoryScene);
    require(imagination.post_route_forfeit == StoryMatchPostRoute::StoryScene);
    require(imagination.post_route_completed_trigger_wipe);
    require(imagination.post_route_forfeit_trigger_wipe);

    const StoryMatchPolicyDescriptor& tix_lunch = story_match_policy_for_kind(StoryMatchKind::TixLunch);
    require(tix_lunch.match_kind == StoryMatchKind::TixLunch);
    require(tix_lunch.scenario_id == StoryMatchScenarioId::TixLunch);
    require(tix_lunch.score_model == StoryMatchScoreModel::SingleGame);
    require(tix_lunch.exit_behavior == StoryMatchExitBehavior::Forfeit);
    require(tix_lunch.post_route_completed == StoryMatchPostRoute::StoryScene);
    require(tix_lunch.post_route_forfeit == StoryMatchPostRoute::StoryHub);
    require(!tix_lunch.post_route_completed_trigger_wipe);
    require(!tix_lunch.post_route_forfeit_trigger_wipe);
}

void test_intro_first_match_policy_defaults() {
    using namespace whacker::app;

    const StoryMatchPolicyDescriptor& intro_policy = story_intro_first_match_policy();
    require(intro_policy.scenario_id == StoryMatchScenarioId::IntroFirstMatch);
    require(intro_policy.progression.xp_enabled);
    require(intro_policy.progression.xp_on_forfeit);
    require(intro_policy.exit_behavior == StoryMatchExitBehavior::Forfeit);
    require(intro_policy.exit_requires_confirmation);
    require(intro_policy.forfeit_unlock_balls == 4);
    require(intro_policy.post_route_completed_trigger_wipe);
    require(intro_policy.post_route_forfeit_trigger_wipe);
}

void test_onboarding_transition_wipe_directives() {
    using namespace whacker::app;

    require(
        story_onboarding_transition_triggers_wipe(
            StoryOnboardingStep::CoachBriefScene,
            StoryOnboardingStep::AtHomeYoutubeScene));
    require(
        !story_onboarding_transition_triggers_wipe(
            StoryOnboardingStep::ClubIntroScene,
            StoryOnboardingStep::EntryBenchmarkMatch));
}

void test_runtime_policy_resolution_prefers_intro_first_match_when_active() {
    using namespace whacker::app;

    StoryRuntimeState runtime {};
    StoryIntroState intro {};
    MatchFlowState flow {};
    AppState app_state = AppState::StoryIntro;
    const AppState pause_return_state = AppState::Playing;

    intro.phase = StoryIntroPhase::PlayMatch;
    runtime.active_match = StoryMatchKind::None;
    flow.mode = ActiveMatchMode::StoryTraining;
    const StoryMatchPolicyDescriptor& intro_policy =
        story_match_policy_for_runtime(runtime, intro, app_state, pause_return_state, flow);
    require(intro_policy.scenario_id == StoryMatchScenarioId::IntroFirstMatch);

    runtime.active_match = StoryMatchKind::Official;
    app_state = AppState::Playing;
    const StoryMatchPolicyDescriptor& official_policy =
        story_match_policy_for_runtime(runtime, intro, app_state, pause_return_state, flow);
    require(official_policy.scenario_id == StoryMatchScenarioId::Official);
}

}  // namespace

int main() {
    test_story_match_policy_catalog_defaults();
    test_intro_first_match_policy_defaults();
    test_onboarding_transition_wipe_directives();
    test_runtime_policy_resolution_prefers_intro_first_match_when_active();
    return 0;
}
