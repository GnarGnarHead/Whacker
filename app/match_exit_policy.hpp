#pragma once

#include <cstdint>
#include <string_view>

#include "match_flow.hpp"
#include "story_match.hpp"
#include "story_script_catalog.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

enum class MatchSessionKind : std::uint8_t {
    None = 0,
    Quick = 1,
    StoryTraining = 2,
    StoryOfficial = 3,
    StoryOnboarding = 4,
    IntroFirstMatch = 5
};

enum class MatchExitAction : std::uint8_t {
    None = 0,
    ExitQuickToSetup = 1,
    ExitStoryMatch = 2,
    ExitIntroContinueStory = 3
};

struct MatchProgress {
    MatchSessionKind session_kind = MatchSessionKind::None;
    StoryMatchKind story_match_kind = StoryMatchKind::None;
    StoryMatchScenarioId scenario_id = StoryMatchScenarioId::None;
    StoryMatchExitBehavior exit_behavior = StoryMatchExitBehavior::None;
    bool exit_requires_confirmation = false;
    std::string_view exit_label {"EXIT MATCH"};
    int forfeit_unlock_balls = 0;
    std::string_view forfeit_unlock_reason {};
    int balls_played = 0;
};

struct MatchExitPolicy {
    bool has_exit_option = false;
    bool can_exit_now = false;
    bool requires_confirmation = false;
    std::string_view exit_label {"EXIT MATCH"};
    std::string_view blocked_reason {};
    MatchExitAction action = MatchExitAction::None;
    StoryMatchEndReason story_end_reason = StoryMatchEndReason::Completed;
};

bool intro_first_match_phase_active(const AppState app_state, int story_intro_phase_value);

MatchProgress make_match_progress(
    AppState active_state,
    const MatchFlowState& match_flow,
    const StoryRuntimeState& story_runtime,
    int story_intro_phase_value,
    int intro_points_played,
    int score_left,
    int score_right);

MatchExitPolicy evaluate_match_exit_policy(const MatchProgress& progress);

}  // namespace whacker::app
