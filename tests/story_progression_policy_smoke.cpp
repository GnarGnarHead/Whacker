#include <cstdlib>

#include "story_intro.hpp"
#include "story_match.hpp"
#include "story_runtime.hpp"
#include "story_skill_limits.hpp"

namespace {

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

whacker::progression::SkillUsageAccumulator seeded_usage() {
    whacker::progression::SkillUsageAccumulator usage {};
    usage.contacts = 12;
    usage.clean_contacts = 6;
    usage.high_edge_contacts = 4;
    usage.sum_abs_u = 6.6f;
    usage.sum_power_samples = 8.4f;
    usage.sum_spin_inject_samples = 5.4f;
    return usage;
}

bool any_skill_component_grew(
    const whacker::progression::SkillState& before,
    const whacker::progression::SkillState& after) {
    return after.edge > before.edge || after.power > before.power || after.spin_inject > before.spin_inject;
}

float total_skill_delta(
    const whacker::progression::SkillState& before,
    const whacker::progression::SkillState& after) {
    return
        (after.edge - before.edge) +
        (after.power - before.power) +
        (after.spin_inject - before.spin_inject);
}

void test_onboarding_aya_completion_awards_growth_without_training_counter_changes() {
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    runtime.active_match = whacker::app::StoryMatchKind::OnboardingAyaFriendly;
    runtime.player_usage = seeded_usage();

    const whacker::progression::SkillState skills_before = runtime.career.player_skills;
    const whacker::progression::SkillState caps_before = runtime.career.player_skill_caps;

    whacker::sim::RallyState terminal {};
    terminal.left_score = 11;
    terminal.right_score = 8;
    whacker::app::finalize_story_match(
        runtime,
        hub,
        terminal,
        nullptr,
        whacker::app::StoryMatchEndReason::Completed);

    require(any_skill_component_grew(skills_before, runtime.career.player_skills));
    require(any_skill_component_grew(caps_before, runtime.career.player_skill_caps));
    require(total_skill_delta(skills_before, runtime.career.player_skills) >= 0.015f);
    require(total_skill_delta(caps_before, runtime.career.player_skill_caps) >= 0.015f);
    require(runtime.career.training_used == 0);
    require(runtime.career.training_matches_played == 0);
}

void test_onboarding_entry_forfeit_awards_partial_growth() {
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    runtime.active_match = whacker::app::StoryMatchKind::OnboardingEntry;
    runtime.player_usage = seeded_usage();

    const whacker::progression::SkillState skills_before = runtime.career.player_skills;
    const whacker::progression::SkillState caps_before = runtime.career.player_skill_caps;

    whacker::sim::RallyState terminal {};
    terminal.left_score = 2;
    terminal.right_score = 1;
    whacker::app::finalize_story_match(
        runtime,
        hub,
        terminal,
        nullptr,
        whacker::app::StoryMatchEndReason::Forfeit);

    require(any_skill_component_grew(skills_before, runtime.career.player_skills));
    require(any_skill_component_grew(caps_before, runtime.career.player_skill_caps));
    require(total_skill_delta(skills_before, runtime.career.player_skills) >= 0.007f);
    require(total_skill_delta(caps_before, runtime.career.player_skill_caps) >= 0.007f);
    require(runtime.career.training_used == 0);
    require(runtime.career.training_matches_played == 0);
}

void test_intro_completion_awards_growth() {
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    whacker::app::StoryIntroState intro {};
    whacker::app::MatchFlowState flow {};
    whacker::sim::Simulation simulation {};
    whacker::app::AppState app_state = whacker::app::AppState::StoryIntro;
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};

    intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    intro.entered_name = "PLAYER";
    intro.player_won = true;
    intro.player_is_right = false;
    intro.final_left_score = 11;
    intro.final_right_score = 7;
    intro.player_usage = seeded_usage();

    const whacker::progression::SkillState skills_before = runtime.career.player_skills;
    const whacker::progression::SkillState caps_before = runtime.career.player_skill_caps;

    whacker::app::complete_story_intro(
        runtime,
        hub,
        intro,
        flow,
        simulation,
        app_state,
        authored_transition_request,
        nullptr,
        nullptr);

    require(any_skill_component_grew(skills_before, runtime.career.player_skills));
    require(any_skill_component_grew(caps_before, runtime.career.player_skill_caps));
    require(total_skill_delta(skills_before, runtime.career.player_skills) >= 0.015f);
    require(total_skill_delta(caps_before, runtime.career.player_skill_caps) >= 0.015f);
    require(app_state == whacker::app::AppState::StoryScene);
    require(authored_transition_request.armed);
}

void test_imagination_match_records_tix_result_without_counting_weekly_official() {
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    runtime.active_match = whacker::app::StoryMatchKind::Imagination1967;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::Imagination1967Match;
    runtime.player_usage = seeded_usage();
    runtime.career.prefers_right_side = false;
    runtime.career.official_completed = true;

    whacker::sim::RallyState terminal {};
    terminal.left_score = 11;
    terminal.right_score = 9;
    whacker::app::finalize_story_match(
        runtime,
        hub,
        terminal,
        nullptr,
        whacker::app::StoryMatchEndReason::Completed);

    require(runtime.career.tix_1967_seen);
    require(runtime.career.tix_1967_player_won);
    require(runtime.career.tix_1967_score_for == 11);
    require(runtime.career.tix_1967_score_against == 9);
    require(runtime.onboarding_step == whacker::app::StoryOnboardingStep::TixMidweekScene);
    require(runtime.career.joined_club);
    require(!runtime.career.official_completed);
    require(!runtime.career.progression_node_id.empty());
}

}  // namespace

int main() {
    test_onboarding_aya_completion_awards_growth_without_training_counter_changes();
    test_onboarding_entry_forfeit_awards_partial_growth();
    test_intro_completion_awards_growth();
    test_imagination_match_records_tix_result_without_counting_weekly_official();
    return 0;
}
