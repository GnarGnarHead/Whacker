#pragma once

#include <cassert>
#include <cstdint>
#include <string_view>

#include "match_flow.hpp"
#include "progression/skills.hpp"
#include "story_rivals.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct StorySceneState;
struct StoryIntroState;

enum class StoryMatchScenarioId : std::uint8_t {
    None = 0,
    IntroFirstMatch = 1,
    OnboardingAyaFriendly = 2,
    OnboardingEntry = 3,
    Training = 4,
    Official = 5,
    Imagination1967 = 6,
    TixLunch = 7,
};

enum class StoryMatchScoreModel : std::uint8_t {
    None = 0,
    RallyLoop = 1,
    SingleGame = 2,
    BestOfGames = 3,
};

enum class StoryMatchExitBehavior : std::uint8_t {
    None = 0,
    StopTraining = 1,
    Forfeit = 2,
};

enum class StoryMatchPostRoute : std::uint8_t {
    StoryHub = 0,
    OnboardingClubIntroScene = 1,
    OnboardingCoachBriefScene = 2,
    OnboardingEntryRetryScene = 3,
    PostForfeitSupportScene = 4,
    StoryScene = 5,
};

enum class StoryGraphNodeKind : std::uint8_t {
    Hub = 0,
    Credits = 1,
};

struct StoryGraphNodeSpec {
    std::string_view node_id {};
    StoryGraphNodeKind kind = StoryGraphNodeKind::Hub;
    int display_week = 1;
    StoryRivalSpec training_rival {};
    StoryRivalSpec official_rival {};
    std::string_view next_node_id {};
};

struct StoryMatchProgressionPolicy {
    bool xp_enabled = false;
    bool xp_on_forfeit = false;
};

struct StoryMatchCounterPolicy {
    bool increment_training_used = false;
    bool increment_training_matches_played = false;
    bool mark_official_completed = false;
    bool update_official_record = false;
    bool update_official_forfeit_streak = false;
    bool update_reputation = false;
};

struct StoryMatchNarrativePolicy {
    bool update_onboarding_hints = false;
    bool update_onboarding_aya_feedback = false;
    bool use_training_feedback = false;
    bool use_official_feedback = false;
};

struct StoryMatchPolicyDescriptor {
    StoryMatchScenarioId scenario_id = StoryMatchScenarioId::None;
    StoryMatchKind match_kind = StoryMatchKind::None;
    ActiveMatchMode session_mode = ActiveMatchMode::StoryTraining;
    StoryMatchScoreModel score_model = StoryMatchScoreModel::None;
    int games_to_win = 0;
    bool ai_training_context = false;
    int ai_preview_points = 0;

    StoryMatchExitBehavior exit_behavior = StoryMatchExitBehavior::None;
    bool exit_requires_confirmation = false;
    std::string_view exit_label {"EXIT MATCH"};
    int forfeit_unlock_balls = 0;
    std::string_view forfeit_unlock_reason {};

    StoryMatchPostRoute post_route_completed = StoryMatchPostRoute::StoryHub;
    StoryMatchPostRoute post_route_forfeit = StoryMatchPostRoute::StoryHub;
    bool post_route_completed_trigger_wipe = false;
    bool post_route_forfeit_trigger_wipe = false;
    StoryOnboardingStep retry_step_if_forfeit = StoryOnboardingStep::None;
    bool prepend_forfeit_feedback = false;

    StoryMatchProgressionPolicy progression {};
    StoryMatchCounterPolicy counters {};
    StoryMatchNarrativePolicy narrative {};
};

const StoryRivalSpec& story_script_intro_rival_spec();
const whacker::progression::SkillState& story_script_imagination_1967_player_skills();
const whacker::progression::SkillState& story_script_imagination_1967_rival_skills();
StoryRivalId story_script_training_rival_for_week(int week);
StoryRivalId story_script_official_rival_for_week(int week);
const StoryRivalSpec& story_script_match_spec(StoryMatchKind match_kind, int week);
const StoryGraphNodeSpec& story_graph_start_node();
const StoryGraphNodeSpec* story_graph_node(std::string_view node_id);
const StoryGraphNodeSpec& story_graph_node_for_career(const StoryCareerData& career);
bool story_graph_has_next_node(const StoryCareerData& career);
bool story_graph_initialize_career_node(StoryCareerData& career);
bool story_graph_advance_career_node(StoryCareerData& career);
bool story_graph_all_authored_edges_resolve();
const StoryMatchPolicyDescriptor& story_match_policy_fallback();
const StoryMatchPolicyDescriptor& story_match_policy_for_kind(StoryMatchKind match_kind);
const StoryMatchPolicyDescriptor& story_intro_first_match_policy();
inline bool story_policy_post_route_triggers_wipe(const StoryMatchPolicyDescriptor& policy, const bool forfeiting) {
    return forfeiting ? policy.post_route_forfeit_trigger_wipe : policy.post_route_completed_trigger_wipe;
}

inline bool story_onboarding_transition_triggers_wipe(
    const StoryOnboardingStep from_step,
    const StoryOnboardingStep to_step) {
    switch (from_step) {
        case StoryOnboardingStep::EarlyArrivalScene:
            if (to_step == StoryOnboardingStep::AyaFriendlyMatch) {
                return false;
            }
            break;
        case StoryOnboardingStep::ClubIntroScene:
            if (to_step == StoryOnboardingStep::EntryBenchmarkMatch) {
                return false;
            }
            break;
        case StoryOnboardingStep::CoachBriefScene:
            if (to_step == StoryOnboardingStep::AtHomeYoutubeScene) {
                return true;
            }
            break;
        case StoryOnboardingStep::AtHomeYoutubeScene:
            if (to_step == StoryOnboardingStep::Imagination1967Match) {
                return false;
            }
            break;
        case StoryOnboardingStep::TixMidweekScene:
            if (to_step == StoryOnboardingStep::Complete) {
                return false;
            }
            break;
        case StoryOnboardingStep::PostTixLunchScene:
            if (to_step == StoryOnboardingStep::Complete) {
                return true;
            }
            break;
        case StoryOnboardingStep::EntryRetryScene:
            if (to_step == StoryOnboardingStep::EntryBenchmarkMatch) {
                return false;
            }
            break;
        case StoryOnboardingStep::None:
        case StoryOnboardingStep::AyaFriendlyMatch:
        case StoryOnboardingStep::EntryBenchmarkMatch:
        case StoryOnboardingStep::Complete:
        case StoryOnboardingStep::Imagination1967Match:
        default:
            break;
    }
#ifndef NDEBUG
    assert(false && "Missing authored wipe directive for onboarding transition.");
#endif
    return false;
}
const StoryRivalSpec& story_policy_rival_spec_for_week(const StoryMatchPolicyDescriptor& policy, int week);
const StoryRivalSpec& story_policy_rival_spec_for_career(
    const StoryMatchPolicyDescriptor& policy,
    const StoryCareerData& career);

const StoryMatchPolicyDescriptor& story_match_policy_for_runtime(
    const StoryRuntimeState& story_runtime,
    const StoryIntroState& story_intro_state,
    AppState active_state,
    const MatchFlowState& match_flow);

void populate_story_onboarding_scene_script(
    StorySceneState& scene_state,
    const StoryRuntimeState& story_runtime);

}  // namespace whacker::app
