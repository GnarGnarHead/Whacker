#include <array>
#include <cmath>
#include <cstdlib>
#include <string>

#include "story_rivals.hpp"
#include "story_script_catalog.hpp"

namespace {

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

float skill_sum(const whacker::progression::SkillState& skills) {
    return skills.edge + skills.power + skills.spin_inject;
}

void require_skill_component_bounds(const whacker::progression::SkillState& skills) {
    require(skills.edge >= 0.0f && skills.edge <= 1.0f);
    require(skills.power >= 0.0f && skills.power <= 1.0f);
    require(skills.spin_inject >= 0.0f && skills.spin_inject <= 1.0f);
}

void require_skill_budget_bounds(const whacker::progression::SkillState& skills) {
    constexpr float kBudgetCap = whacker::progression::kSkillBudgetCap;
    require(skill_sum(skills) <= kBudgetCap + 1.0e-5f);
}

void test_story_rival_specs_obey_skill_bounds_and_budget() {
    const std::array<whacker::app::StoryRivalId, 11> rival_ids = {
        whacker::app::StoryRivalId::Kai,
        whacker::app::StoryRivalId::Aya,
        whacker::app::StoryRivalId::Benji,
        whacker::app::StoryRivalId::Tix,
        whacker::app::StoryRivalId::Issa,
        whacker::app::StoryRivalId::Jolo,
        whacker::app::StoryRivalId::Juno,
        whacker::app::StoryRivalId::Rook,
        whacker::app::StoryRivalId::Mira,
        whacker::app::StoryRivalId::Vex,
        whacker::app::StoryRivalId::Nova,
    };

    for (const whacker::app::StoryRivalId id : rival_ids) {
        const whacker::app::StoryRivalSpec& spec = whacker::app::story_rival_spec(id);
        require(spec.id == id);
        require(spec.name != nullptr);
        require_skill_component_bounds(spec.skills);
        require_skill_budget_bounds(spec.skills);
    }

    const whacker::app::StoryRivalSpec& none = whacker::app::story_rival_spec(whacker::app::StoryRivalId::None);
    require(none.id == whacker::app::StoryRivalId::None);
    require_skill_component_bounds(none.skills);
    require_skill_budget_bounds(none.skills);
}

void test_story_intro_and_onboarding_rival_routes() {
    const whacker::app::StoryRivalSpec& intro = whacker::app::story_script_intro_rival_spec();
    require(intro.id == whacker::app::StoryRivalId::Kai);

    const whacker::app::StoryRivalSpec& onboarding_friendly = whacker::app::story_script_match_spec(
        whacker::app::StoryMatchKind::OnboardingAyaFriendly,
        1);
    require(onboarding_friendly.id == whacker::app::StoryRivalId::Aya);
    require(onboarding_friendly.skills.edge == 0.17f);
    require(onboarding_friendly.skills.power == 0.12f);
    require(onboarding_friendly.skills.spin_inject == 0.12f);
    require_skill_budget_bounds(onboarding_friendly.skills);

    const whacker::app::StoryRivalSpec& onboarding_entry = whacker::app::story_script_match_spec(
        whacker::app::StoryMatchKind::OnboardingEntry,
        1);
    require(onboarding_entry.id == whacker::app::StoryRivalId::Benji);
    require(onboarding_entry.skills.edge == 0.02f);
    require(onboarding_entry.skills.power == 0.04f);
    require(onboarding_entry.skills.spin_inject == 0.40f);
    require_skill_budget_bounds(onboarding_entry.skills);
}

void test_imagination_1967_champion_profiles_use_spin_heavy_full_budget() {
    const whacker::progression::SkillState champion_player =
        whacker::app::story_script_imagination_1967_player_skills();
    const whacker::progression::SkillState champion_rival =
        whacker::app::story_script_imagination_1967_rival_skills();

    require_skill_component_bounds(champion_player);
    require_skill_component_bounds(champion_rival);
    require_skill_budget_bounds(champion_player);
    require_skill_budget_bounds(champion_rival);
    require(std::fabs(skill_sum(champion_player) - whacker::progression::kSkillBudgetCap) <= 1.0e-5f);
    require(std::fabs(skill_sum(champion_rival) - whacker::progression::kSkillBudgetCap) <= 1.0e-5f);
    require(champion_player.spin_inject >= champion_player.edge);
    require(champion_player.spin_inject >= champion_player.power);
    require(champion_rival.spin_inject >= champion_rival.edge);
    require(champion_rival.spin_inject >= champion_rival.power);
}

void test_story_weekly_training_and_official_routing() {
    constexpr whacker::app::StoryRivalId kExpectedTraining = whacker::app::StoryRivalId::Kai;
    constexpr whacker::app::StoryRivalId kExpectedOfficial = whacker::app::StoryRivalId::Aya;
    const std::array<int, 5> weeks = {0, 1, 2, 6, 999};

    for (const int week : weeks) {
        const whacker::app::StoryRivalId training_id = whacker::app::story_script_training_rival_for_week(week);
        const whacker::app::StoryRivalId official_id = whacker::app::story_script_official_rival_for_week(week);
        require(training_id == kExpectedTraining);
        require(official_id == kExpectedOfficial);

        const whacker::app::StoryRivalSpec& training_match =
            whacker::app::story_script_match_spec(whacker::app::StoryMatchKind::Training, week);
        const whacker::app::StoryRivalSpec& official_match =
            whacker::app::story_script_match_spec(whacker::app::StoryMatchKind::Official, week);
        require(training_match.id == training_id);
        require(official_match.id == official_id);
        require_skill_budget_bounds(training_match.skills);
        require_skill_budget_bounds(official_match.skills);
    }
}

void test_story_graph_progression_stops_at_authored_terminal() {
    whacker::app::StoryCareerData career {};
    career.joined_club = true;
    career.week = 1;
    require(whacker::app::story_graph_initialize_career_node(career));
    require(career.progression_node_id == std::string("club_week_01"));
    require(!whacker::app::story_graph_has_next_node(career));

    int advance_count = 0;
    while (whacker::app::story_graph_advance_career_node(career)) {
        ++advance_count;
    }
    require(advance_count == 0);
    require(career.progression_node_id == std::string("club_week_01"));
    require(!whacker::app::story_graph_advance_career_node(career));
}

}  // namespace

int main() {
    test_story_rival_specs_obey_skill_bounds_and_budget();
    test_story_intro_and_onboarding_rival_routes();
    test_imagination_1967_champion_profiles_use_spin_heavy_full_budget();
    test_story_weekly_training_and_official_routing();
    test_story_graph_progression_stops_at_authored_terminal();
    return 0;
}
