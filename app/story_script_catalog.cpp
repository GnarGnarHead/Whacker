#include "story_script_catalog.hpp"

#include <cassert>
#include <algorithm>
#include <array>
#include <cstddef>

#include "story_pack.hpp"
#include "story_intro.hpp"
#include "story_scene.hpp"
#include "story_text.hpp"

namespace whacker::app {

namespace {

std::span<const StoryGraphNodeSpec> story_hub_graph() {
    const std::span<const StoryGraphNodeSpec> graph = story_pack::season1_hub_graph();
#ifndef NDEBUG
    assert(!graph.empty() && "Season 1 hub graph pack is empty.");
#endif
    return graph;
}

constexpr StoryRivalSpec kStoryScriptIntroRival {
    .id = StoryRivalId::Kai,
    .name = kStoryRivalKai.name,
    .style = AiStyle::Balanced,
    .skills = {.edge = 0.12f, .power = 0.12f, .spin_inject = 0.12f},
};

constexpr StoryRivalSpec kStoryScriptOnboardingAyaFriendly {
    .id = StoryRivalId::Aya,
    .name = kStoryRivalAya.name,
    .style = AiStyle::Balanced,
    .skills = {.edge = 0.17f, .power = 0.12f, .spin_inject = 0.12f},
};

constexpr StoryRivalSpec kStoryScriptOnboardingEntry {
    .id = StoryRivalId::Benji,
    .name = kStoryRivalBenji.name,
    .style = AiStyle::Spin,
    .skills = {.edge = 0.02f, .power = 0.04f, .spin_inject = 0.40f},
};

constexpr whacker::progression::SkillState kStoryScriptImagination1967PlayerSkills {
    .edge = 0.38f,
    .power = 0.60f,
    .spin_inject = 0.72f,
};

constexpr whacker::progression::SkillState kStoryScriptImagination1967RivalSkills {
    .edge = 0.42f,
    .power = 0.54f,
    .spin_inject = 0.74f,
};

constexpr StoryRivalSpec kStoryScriptImagination1967 {
    .id = StoryRivalId::None,
    .name = "1967 FINAL",
    .style = AiStyle::Spin,
    .skills = kStoryScriptImagination1967RivalSkills,
};

constexpr StoryRivalSpec kStoryScriptTixLunch {
    .id = StoryRivalId::Tix,
    .name = kStoryRivalTix.name,
    .style = AiStyle::Technical,
    .skills = {.edge = 0.30f, .power = 0.22f, .spin_inject = 0.32f},
};

constexpr StoryMatchPolicyDescriptor kStoryMatchPolicyFallback {
    .scenario_id = StoryMatchScenarioId::None,
    .match_kind = StoryMatchKind::None,
    .session_mode = ActiveMatchMode::StoryTraining,
    .score_model = StoryMatchScoreModel::None,
    .games_to_win = 0,
    .ai_training_context = false,
    .exit_behavior = StoryMatchExitBehavior::None,
    .exit_requires_confirmation = false,
    .exit_label = "EXIT MATCH",
    .forfeit_unlock_balls = 0,
    .forfeit_unlock_reason = {},
    .post_route_completed = StoryMatchPostRoute::StoryHub,
    .post_route_forfeit = StoryMatchPostRoute::StoryHub,
    .post_route_completed_trigger_wipe = false,
    .post_route_forfeit_trigger_wipe = false,
    .retry_step_if_forfeit = StoryOnboardingStep::None,
    .prepend_forfeit_feedback = false,
    .progression = {.xp_enabled = false, .xp_on_forfeit = false},
    .counters = {},
    .narrative = {},
};

constexpr StoryMatchPolicyDescriptor kStoryMatchPolicyIntroFirstMatch {
    .scenario_id = StoryMatchScenarioId::IntroFirstMatch,
    .match_kind = StoryMatchKind::None,
    .session_mode = ActiveMatchMode::StoryTraining,
    .score_model = StoryMatchScoreModel::SingleGame,
    .games_to_win = 1,
    .ai_training_context = false,
    .exit_behavior = StoryMatchExitBehavior::Forfeit,
    .exit_requires_confirmation = true,
    .exit_label = "FORFEIT MATCH",
    .forfeit_unlock_balls = 4,
    .forfeit_unlock_reason = "FORFEIT UNLOCKS AFTER BALL 4",
    .post_route_completed = StoryMatchPostRoute::StoryScene,
    .post_route_forfeit = StoryMatchPostRoute::StoryScene,
    .post_route_completed_trigger_wipe = true,
    .post_route_forfeit_trigger_wipe = true,
    .retry_step_if_forfeit = StoryOnboardingStep::None,
    .prepend_forfeit_feedback = false,
    .progression = {.xp_enabled = true, .xp_on_forfeit = true},
    .counters = {},
    .narrative = {},
};

constexpr StoryMatchPolicyDescriptor kStoryMatchPolicyOnboardingAya {
    .scenario_id = StoryMatchScenarioId::OnboardingAyaFriendly,
    .match_kind = StoryMatchKind::OnboardingAyaFriendly,
    .session_mode = ActiveMatchMode::StoryTraining,
    .score_model = StoryMatchScoreModel::SingleGame,
    .games_to_win = 1,
    .ai_training_context = false,
    .exit_behavior = StoryMatchExitBehavior::Forfeit,
    .exit_requires_confirmation = true,
    .exit_label = "FORFEIT MATCH",
    .forfeit_unlock_balls = 0,
    .forfeit_unlock_reason = {},
    .post_route_completed = StoryMatchPostRoute::OnboardingClubIntroScene,
    .post_route_forfeit = StoryMatchPostRoute::OnboardingClubIntroScene,
    .post_route_completed_trigger_wipe = false,
    .post_route_forfeit_trigger_wipe = false,
    .retry_step_if_forfeit = StoryOnboardingStep::None,
    .prepend_forfeit_feedback = false,
    .progression = {.xp_enabled = true, .xp_on_forfeit = true},
    .counters = {},
    .narrative = {
        .update_onboarding_hints = true,
        .update_onboarding_aya_feedback = true,
        .use_training_feedback = false,
        .use_official_feedback = false,
    },
};

constexpr StoryMatchPolicyDescriptor kStoryMatchPolicyOnboardingEntry {
    .scenario_id = StoryMatchScenarioId::OnboardingEntry,
    .match_kind = StoryMatchKind::OnboardingEntry,
    .session_mode = ActiveMatchMode::StoryTraining,
    .score_model = StoryMatchScoreModel::SingleGame,
    .games_to_win = 1,
    .ai_training_context = false,
    .exit_behavior = StoryMatchExitBehavior::None,
    .exit_requires_confirmation = false,
    .exit_label = "FORFEIT MATCH",
    .forfeit_unlock_balls = 0,
    .forfeit_unlock_reason = {},
    .post_route_completed = StoryMatchPostRoute::OnboardingCoachBriefScene,
    .post_route_forfeit = StoryMatchPostRoute::OnboardingEntryRetryScene,
    .post_route_completed_trigger_wipe = false,
    .post_route_forfeit_trigger_wipe = false,
    .retry_step_if_forfeit = StoryOnboardingStep::EntryRetryScene,
    .prepend_forfeit_feedback = false,
    .progression = {.xp_enabled = true, .xp_on_forfeit = true},
    .counters = {},
    .narrative = {
        .update_onboarding_hints = false,
        .update_onboarding_aya_feedback = false,
        .use_training_feedback = false,
        .use_official_feedback = false,
    },
};

constexpr StoryMatchPolicyDescriptor kStoryMatchPolicyImagination1967 {
    .scenario_id = StoryMatchScenarioId::Imagination1967,
    .match_kind = StoryMatchKind::Imagination1967,
    .session_mode = ActiveMatchMode::StoryTraining,
    .score_model = StoryMatchScoreModel::SingleGame,
    .games_to_win = 1,
    .ai_training_context = false,
    .ai_preview_points = 4,
    .exit_behavior = StoryMatchExitBehavior::None,
    .exit_requires_confirmation = false,
    .exit_label = "EXIT MATCH",
    .forfeit_unlock_balls = 0,
    .forfeit_unlock_reason = {},
    .post_route_completed = StoryMatchPostRoute::StoryScene,
    .post_route_forfeit = StoryMatchPostRoute::StoryScene,
    .post_route_completed_trigger_wipe = true,
    .post_route_forfeit_trigger_wipe = true,
    .retry_step_if_forfeit = StoryOnboardingStep::None,
    .prepend_forfeit_feedback = false,
    .progression = {.xp_enabled = true, .xp_on_forfeit = true},
    .counters = {},
    .narrative = {
        .update_onboarding_hints = true,
        .update_onboarding_aya_feedback = false,
        .use_training_feedback = false,
        .use_official_feedback = false,
    },
};

constexpr StoryMatchPolicyDescriptor kStoryMatchPolicyTixLunch {
    .scenario_id = StoryMatchScenarioId::TixLunch,
    .match_kind = StoryMatchKind::TixLunch,
    .session_mode = ActiveMatchMode::StoryTraining,
    .score_model = StoryMatchScoreModel::SingleGame,
    .games_to_win = 1,
    .ai_training_context = false,
    .ai_preview_points = 0,
    .exit_behavior = StoryMatchExitBehavior::Forfeit,
    .exit_requires_confirmation = true,
    .exit_label = "END LUNCH MATCH",
    .forfeit_unlock_balls = 0,
    .forfeit_unlock_reason = {},
    .post_route_completed = StoryMatchPostRoute::StoryScene,
    .post_route_forfeit = StoryMatchPostRoute::StoryHub,
    .post_route_completed_trigger_wipe = false,
    .post_route_forfeit_trigger_wipe = false,
    .retry_step_if_forfeit = StoryOnboardingStep::None,
    .prepend_forfeit_feedback = false,
    .progression = {.xp_enabled = true, .xp_on_forfeit = true},
    .counters = {},
    .narrative = {
        .update_onboarding_hints = false,
        .update_onboarding_aya_feedback = false,
        .use_training_feedback = false,
        .use_official_feedback = false,
    },
};

constexpr StoryMatchPolicyDescriptor kStoryMatchPolicyTraining {
    .scenario_id = StoryMatchScenarioId::Training,
    .match_kind = StoryMatchKind::Training,
    .session_mode = ActiveMatchMode::StoryTraining,
    .score_model = StoryMatchScoreModel::RallyLoop,
    .games_to_win = 0,
    .ai_training_context = true,
    .exit_behavior = StoryMatchExitBehavior::StopTraining,
    .exit_requires_confirmation = false,
    .exit_label = "STOP TRAINING",
    .forfeit_unlock_balls = 0,
    .forfeit_unlock_reason = {},
    .post_route_completed = StoryMatchPostRoute::StoryHub,
    .post_route_forfeit = StoryMatchPostRoute::StoryHub,
    .post_route_completed_trigger_wipe = false,
    .post_route_forfeit_trigger_wipe = false,
    .retry_step_if_forfeit = StoryOnboardingStep::None,
    .prepend_forfeit_feedback = false,
    .progression = {.xp_enabled = true, .xp_on_forfeit = true},
    .counters = {
        .increment_training_used = true,
        .increment_training_matches_played = true,
        .mark_official_completed = false,
        .update_official_record = false,
        .update_official_forfeit_streak = false,
        .update_reputation = false,
    },
    .narrative = {
        .update_onboarding_hints = false,
        .update_onboarding_aya_feedback = false,
        .use_training_feedback = true,
        .use_official_feedback = false,
    },
};

constexpr StoryMatchPolicyDescriptor kStoryMatchPolicyOfficial {
    .scenario_id = StoryMatchScenarioId::Official,
    .match_kind = StoryMatchKind::Official,
    .session_mode = ActiveMatchMode::StoryOfficial,
    .score_model = StoryMatchScoreModel::BestOfGames,
    .games_to_win = 0,
    .ai_training_context = false,
    .exit_behavior = StoryMatchExitBehavior::Forfeit,
    .exit_requires_confirmation = true,
    .exit_label = "FORFEIT MATCH",
    .forfeit_unlock_balls = 0,
    .forfeit_unlock_reason = {},
    .post_route_completed = StoryMatchPostRoute::StoryHub,
    .post_route_forfeit = StoryMatchPostRoute::PostForfeitSupportScene,
    .post_route_completed_trigger_wipe = false,
    .post_route_forfeit_trigger_wipe = false,
    .retry_step_if_forfeit = StoryOnboardingStep::None,
    .prepend_forfeit_feedback = false,
    .progression = {.xp_enabled = true, .xp_on_forfeit = true},
    .counters = {
        .increment_training_used = false,
        .increment_training_matches_played = false,
        .mark_official_completed = true,
        .update_official_record = true,
        .update_official_forfeit_streak = true,
        .update_reputation = true,
    },
    .narrative = {
        .update_onboarding_hints = false,
        .update_onboarding_aya_feedback = false,
        .use_training_feedback = false,
        .use_official_feedback = true,
    },
};

static_assert(story_rival_spec_valid(kStoryScriptIntroRival), "Story script intro rival violates skill constraints.");
static_assert(
    story_rival_spec_valid(kStoryScriptOnboardingAyaFriendly),
    "Story script onboarding Aya friendly rival violates skill constraints.");
static_assert(
    story_rival_spec_valid(kStoryScriptOnboardingEntry),
    "Story script onboarding entry rival violates skill constraints.");
static_assert(
    story_rival_spec_valid(kStoryScriptImagination1967),
    "Story script imagination 1967 rival violates skill constraints.");
static_assert(
    story_rival_spec_valid(kStoryScriptTixLunch),
    "Story script Tix lunch rival violates skill constraints.");

std::size_t week_schedule_index(const int week, const std::size_t count) {
    const int safe_week = week < 1 ? 1 : week;
    const int clamped = std::clamp(safe_week - 1, 0, static_cast<int>(count) - 1);
    return static_cast<std::size_t>(clamped);
}

const StoryGraphNodeSpec* story_graph_node_impl(const std::string_view node_id) {
    if (node_id.empty()) {
        return nullptr;
    }
    const std::span<const StoryGraphNodeSpec> graph = story_hub_graph();
    for (const StoryGraphNodeSpec& node : graph) {
        if (node.node_id == node_id) {
            return &node;
        }
    }
    return nullptr;
}

const StoryGraphNodeSpec& story_graph_node_for_week_impl(const int week) {
    const std::span<const StoryGraphNodeSpec> graph = story_hub_graph();
    return graph[week_schedule_index(week, graph.size())];
}

const StoryGraphNodeSpec& story_graph_node_for_career_impl(const StoryCareerData& career) {
    if (const StoryGraphNodeSpec* node = story_graph_node_impl(career.progression_node_id)) {
        return *node;
    }
    return story_graph_node_for_week_impl(career.week);
}

void push_line(
    StorySceneState& scene_state,
    const std::string& line,
    const StorySceneSpeaker speaker = StorySceneSpeaker::Rival,
    const StoryPortraitId portrait_id = StoryPortraitId::None) {
    if (scene_state.line_count >= static_cast<int>(scene_state.lines.size())) {
        return;
    }
    const std::size_t index = static_cast<std::size_t>(scene_state.line_count);
    scene_state.lines[index] = line;
    scene_state.speakers[index] = speaker;
    scene_state.portrait_ids[index] = portrait_id;
    scene_state.line_count += 1;
}

void clear_scene(StorySceneState& scene_state) {
    scene_state = StorySceneState {};
}

void reset_scene_typewriter(StorySceneState& scene_state) {
    scene_state.visible_chars = 0;
    scene_state.type_accum = 0.0f;
    scene_state.typed_line_index = scene_state.line_index;
    scene_state.dialogue_writing = scene_state.id != StorySceneId::None && scene_state.line_count > 0;
    scene_state.scroll_lines_from_bottom = 0;
}

}  // namespace

const StoryRivalSpec& story_script_intro_rival_spec() {
    return kStoryScriptIntroRival;
}

const whacker::progression::SkillState& story_script_imagination_1967_player_skills() {
    return kStoryScriptImagination1967PlayerSkills;
}

const whacker::progression::SkillState& story_script_imagination_1967_rival_skills() {
    return kStoryScriptImagination1967RivalSkills;
}

StoryRivalId story_script_training_rival_for_week(const int week) {
    return story_graph_node_for_week_impl(week).training_rival.id;
}

StoryRivalId story_script_official_rival_for_week(const int week) {
    return story_graph_node_for_week_impl(week).official_rival.id;
}

const StoryRivalSpec& story_script_match_spec(const StoryMatchKind match_kind, const int week) {
    const StoryGraphNodeSpec& graph_node = story_graph_node_for_week_impl(week);
    switch (match_kind) {
        case StoryMatchKind::OnboardingAyaFriendly:
            return kStoryScriptOnboardingAyaFriendly;
        case StoryMatchKind::OnboardingEntry:
            return kStoryScriptOnboardingEntry;
        case StoryMatchKind::Imagination1967:
            return kStoryScriptImagination1967;
        case StoryMatchKind::TixLunch:
            return kStoryScriptTixLunch;
        case StoryMatchKind::Training:
            return graph_node.training_rival;
        case StoryMatchKind::Official:
            return graph_node.official_rival;
        case StoryMatchKind::None:
        default:
            return kStoryScriptIntroRival;
    }
}

const StoryGraphNodeSpec& story_graph_start_node() {
    return story_hub_graph().front();
}

const StoryGraphNodeSpec* story_graph_node(const std::string_view node_id) {
    return story_graph_node_impl(node_id);
}

const StoryGraphNodeSpec& story_graph_node_for_career(const StoryCareerData& career) {
    return story_graph_node_for_career_impl(career);
}

bool story_graph_has_next_node(const StoryCareerData& career) {
    const StoryGraphNodeSpec& node = story_graph_node_for_career_impl(career);
    if (node.kind != StoryGraphNodeKind::Hub) {
        return false;
    }
    return !node.next_node_id.empty() && story_graph_node_impl(node.next_node_id) != nullptr;
}

bool story_graph_initialize_career_node(StoryCareerData& career) {
    if (!career.joined_club) {
        career.progression_node_id.clear();
        return false;
    }
    if (career.story_completed) {
        return false;
    }
    if (story_graph_node_impl(career.progression_node_id) != nullptr) {
        return false;
    }
    const StoryGraphNodeSpec& fallback = story_graph_node_for_week_impl(career.week);
    career.progression_node_id = std::string(fallback.node_id);
    career.week = std::max(career.week, fallback.display_week);
    return true;
}

bool story_graph_advance_career_node(StoryCareerData& career) {
    if (!career.joined_club || career.story_completed) {
        return false;
    }
    (void)story_graph_initialize_career_node(career);
    const StoryGraphNodeSpec* current = story_graph_node_impl(career.progression_node_id);
    if (current == nullptr || current->next_node_id.empty()) {
        return false;
    }
    const StoryGraphNodeSpec* next = story_graph_node_impl(current->next_node_id);
    if (next == nullptr) {
        return false;
    }
    career.progression_node_id = std::string(next->node_id);
    career.week = std::max(career.week + 1, next->display_week);
    return true;
}

bool story_graph_all_authored_edges_resolve() {
    for (const StoryGraphNodeSpec& node : story_hub_graph()) {
        if (!node.next_node_id.empty() && story_graph_node_impl(node.next_node_id) == nullptr) {
            return false;
        }
    }
    return true;
}

const StoryMatchPolicyDescriptor& story_match_policy_fallback() {
    return kStoryMatchPolicyFallback;
}

const StoryMatchPolicyDescriptor& story_match_policy_for_kind(const StoryMatchKind match_kind) {
    switch (match_kind) {
        case StoryMatchKind::OnboardingAyaFriendly:
            return kStoryMatchPolicyOnboardingAya;
        case StoryMatchKind::OnboardingEntry:
            return kStoryMatchPolicyOnboardingEntry;
        case StoryMatchKind::Imagination1967:
            return kStoryMatchPolicyImagination1967;
        case StoryMatchKind::TixLunch:
            return kStoryMatchPolicyTixLunch;
        case StoryMatchKind::Training:
            return kStoryMatchPolicyTraining;
        case StoryMatchKind::Official:
            return kStoryMatchPolicyOfficial;
        case StoryMatchKind::None:
        default:
#ifndef NDEBUG
            assert(false && "Invalid story match kind for policy lookup.");
#endif
            return kStoryMatchPolicyFallback;
    }
}

const StoryMatchPolicyDescriptor& story_intro_first_match_policy() {
    return kStoryMatchPolicyIntroFirstMatch;
}

const StoryRivalSpec& story_policy_rival_spec_for_week(const StoryMatchPolicyDescriptor& policy, const int week) {
    if (policy.scenario_id == StoryMatchScenarioId::IntroFirstMatch) {
        return story_script_intro_rival_spec();
    }
    if (policy.match_kind != StoryMatchKind::None) {
        return story_script_match_spec(policy.match_kind, week);
    }
    return story_rival_spec(StoryRivalId::None);
}

const StoryRivalSpec& story_policy_rival_spec_for_career(
    const StoryMatchPolicyDescriptor& policy,
    const StoryCareerData& career) {
    if (policy.scenario_id == StoryMatchScenarioId::IntroFirstMatch) {
        return story_script_intro_rival_spec();
    }
    if (policy.match_kind == StoryMatchKind::Training || policy.match_kind == StoryMatchKind::Official) {
        const StoryGraphNodeSpec& node = story_graph_node_for_career_impl(career);
        if (policy.match_kind == StoryMatchKind::Training) {
            return node.training_rival;
        }
        return node.official_rival;
    }
    if (policy.match_kind != StoryMatchKind::None) {
        return story_script_match_spec(policy.match_kind, career.week);
    }
    return story_rival_spec(StoryRivalId::None);
}

const StoryMatchPolicyDescriptor& story_match_policy_for_runtime(
    const StoryRuntimeState& story_runtime,
    const StoryIntroState& story_intro_state,
    const AppState app_state,
    const AppState pause_return_state,
    const MatchFlowState& match_flow) {
    const AppState active_state = app_state == AppState::Paused ? pause_return_state : app_state;
    const bool intro_first_match_active =
        active_state == AppState::StoryIntro &&
        story_intro_state.phase >= StoryIntroPhase::PlayMatch &&
        story_intro_state.phase <= StoryIntroPhase::NameEntry &&
        story_runtime.active_match == StoryMatchKind::None &&
        match_flow.mode == ActiveMatchMode::StoryTraining;
    if (intro_first_match_active) {
        return kStoryMatchPolicyIntroFirstMatch;
    }
    if (story_runtime.active_match != StoryMatchKind::None) {
        return story_match_policy_for_kind(story_runtime.active_match);
    }
    return kStoryMatchPolicyFallback;
}

void populate_story_onboarding_scene_script(
    StorySceneState& scene_state,
    const StoryRuntimeState& story_runtime) {
    // Source of truth for onboarding dialogue composition.
    clear_scene(scene_state);
    scene_state.player_is_right = story_runtime.career.prefers_right_side;
    if (story_runtime.post_forfeit_scene_pending) {
        scene_state.id = StorySceneId::PostForfeitSupport;
        scene_state.header = story_text::post_forfeit_scene_header();
        const int streak = std::max(1, story_runtime.career.official_forfeit_streak);
        push_line(
            scene_state,
            story_text::post_forfeit_scene_line_1(streak),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        push_line(
            scene_state,
            story_text::post_forfeit_scene_line_2(streak),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Aya);
        push_line(
            scene_state,
            story_text::post_forfeit_scene_line_3(streak),
            StorySceneSpeaker::Rival,
            streak == 2 ? StoryPortraitId::Tix : StoryPortraitId::CoachReyes);
        reset_scene_typewriter(scene_state);
        return;
    }

    const std::string& player_name = story_runtime.career.player_name;
    if (story_runtime.onboarding_step == StoryOnboardingStep::EarlyArrivalScene) {
        scene_state.id = StorySceneId::OnboardingEarlyArrival;
        scene_state.header = story_text::onboarding_early_arrival_header();
        push_line(
            scene_state,
            story_text::onboarding_aya_early_arrival_line_1(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Aya);
        push_line(
            scene_state,
            story_text::onboarding_aya_early_arrival_line_2(player_name),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Aya);
        push_line(
            scene_state,
            story_text::onboarding_aya_early_arrival_line_3(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Aya);
        push_line(
            scene_state,
            story_text::onboarding_aya_early_arrival_line_4(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Aya);
        reset_scene_typewriter(scene_state);
        return;
    }

    if (story_runtime.onboarding_step == StoryOnboardingStep::ClubIntroScene) {
        scene_state.id = StorySceneId::OnboardingClubIntro;
        scene_state.header = story_text::onboarding_club_floor_header();
        if (story_runtime.onboarding_aya_feedback_available) {
            const std::string line = story_runtime.onboarding_aya_forfeited
                ? story_text::onboarding_aya_forfeit_feedback_line()
                : (story_runtime.onboarding_aya_feedback_from_loss
                       ? story_text::onboarding_aya_guidance_after_loss_line(story_runtime.onboarding_aya_feedback_hint)
                       : story_text::onboarding_aya_guidance_after_win_line(story_runtime.onboarding_aya_feedback_hint));
            push_line(scene_state, line, StorySceneSpeaker::Rival, StoryPortraitId::Aya);
        }
        push_line(
            scene_state,
            story_text::onboarding_aya_intro_to_coach_line(player_name),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Aya);
        push_line(
            scene_state,
            story_text::onboarding_coach_intro_player_line(player_name),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        push_line(
            scene_state,
            story_text::onboarding_coach_welcome_line(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        push_line(
            scene_state,
            story_text::onboarding_coach_assign_benji_line(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        push_line(
            scene_state,
            story_text::onboarding_coach_benji_spin_warning_line(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        reset_scene_typewriter(scene_state);
        return;
    }

    if (story_runtime.onboarding_step == StoryOnboardingStep::CoachBriefScene) {
        scene_state.id = StorySceneId::OnboardingCoachBrief;
        scene_state.header = story_text::onboarding_coach_reyes_header();
        push_line(
            scene_state,
            story_text::onboarding_coach_post_entry_compliment_line(
                story_runtime.onboarding_performance_hint,
                story_runtime.onboarding_style_hint),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        push_line(
            scene_state,
            story_text::onboarding_coach_day_end_line(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        push_line(
            scene_state,
            story_text::onboarding_coach_training_open_line(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        push_line(
            scene_state,
            story_text::onboarding_coach_training_reps_line(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        push_line(
            scene_state,
            story_text::onboarding_tix_post_day_line_1(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        push_line(
            scene_state,
            story_text::onboarding_tix_post_day_line_2(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        push_line(
            scene_state,
            story_text::onboarding_tix_post_day_line_3(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        push_line(
            scene_state,
            story_text::onboarding_tix_post_day_line_4(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        push_line(
            scene_state,
            story_text::onboarding_tix_post_day_line_5(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        reset_scene_typewriter(scene_state);
        return;
    }

    if (story_runtime.onboarding_step == StoryOnboardingStep::AtHomeYoutubeScene) {
        scene_state.id = StorySceneId::PostBenjiAtHomeYoutube;
        scene_state.header = story_text::at_home_youtube_header();
        push_line(
            scene_state,
            story_text::at_home_youtube_line_1(),
            StorySceneSpeaker::None,
            StoryPortraitId::None);
        push_line(
            scene_state,
            story_text::at_home_youtube_line_2(),
            StorySceneSpeaker::None,
            StoryPortraitId::None);
        reset_scene_typewriter(scene_state);
        return;
    }

    if (story_runtime.onboarding_step == StoryOnboardingStep::TixMidweekScene) {
        scene_state.id = StorySceneId::TixMidweekLunchInvite;
        scene_state.header = story_text::tix_midweek_scene_header();
        push_line(
            scene_state,
            story_text::tix_midweek_scene_line_1(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        push_line(
            scene_state,
            story_text::tix_midweek_scene_line_2(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        push_line(
            scene_state,
            story_text::tix_midweek_scene_line_3(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        push_line(
            scene_state,
            story_text::tix_midweek_scene_line_4(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        push_line(
            scene_state,
            story_text::tix_midweek_scene_line_5(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        scene_state.has_binary_choice = true;
        scene_state.binary_choice_yes_selected = true;
        reset_scene_typewriter(scene_state);
        return;
    }

    if (story_runtime.onboarding_step == StoryOnboardingStep::PostTixLunchScene) {
        scene_state.id = StorySceneId::TixPostLunchThanks;
        scene_state.header = story_text::tix_midweek_scene_header();
        push_line(
            scene_state,
            story_text::tix_post_lunch_line_1(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        push_line(
            scene_state,
            story_text::tix_post_lunch_line_2(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::Tix);
        reset_scene_typewriter(scene_state);
        return;
    }

    if (story_runtime.onboarding_step == StoryOnboardingStep::EntryRetryScene) {
        scene_state.id = StorySceneId::OnboardingEntryRetry;
        scene_state.header = story_text::onboarding_coach_reyes_header();
        push_line(
            scene_state,
            story_text::onboarding_coach_entry_retry_line(),
            StorySceneSpeaker::Rival,
            StoryPortraitId::CoachReyes);
        reset_scene_typewriter(scene_state);
        return;
    }

    clear_scene(scene_state);
}

}  // namespace whacker::app
