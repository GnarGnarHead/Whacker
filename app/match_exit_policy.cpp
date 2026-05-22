#include "match_exit_policy.hpp"

#include <algorithm>

namespace whacker::app {

bool intro_first_match_phase_active(const AppState app_state, const int story_intro_phase_value) {
    if (app_state != AppState::StoryIntro) {
        return false;
    }
    // StoryIntroPhase::PlayMatch (1), BetweenBalls (2), NameEntry (3)
    return story_intro_phase_value >= 1 && story_intro_phase_value <= 3;
}

MatchProgress make_match_progress(
    const AppState active_state,
    const MatchFlowState& match_flow,
    const StoryRuntimeState& story_runtime,
    const int story_intro_phase_value,
    const int intro_points_played,
    const int score_left,
    const int score_right) {
    MatchProgress progress {};
    const int balls_played = std::max(0, score_left + score_right);
    const bool intro_first_match_active =
        intro_first_match_phase_active(active_state, story_intro_phase_value) &&
        story_runtime.active_match == StoryMatchKind::None &&
        match_flow.mode == ActiveMatchMode::StoryTraining;

    if (intro_first_match_active) {
        const StoryMatchPolicyDescriptor& intro_policy = story_intro_first_match_policy();
        progress.session_kind = MatchSessionKind::IntroFirstMatch;
        progress.story_match_kind = StoryMatchKind::None;
        progress.scenario_id = intro_policy.scenario_id;
        progress.exit_behavior = intro_policy.exit_behavior;
        progress.exit_requires_confirmation = intro_policy.exit_requires_confirmation;
        progress.exit_label = intro_policy.exit_label;
        progress.forfeit_unlock_balls = intro_policy.forfeit_unlock_balls;
        progress.forfeit_unlock_reason = intro_policy.forfeit_unlock_reason;
        (void)intro_points_played;
        progress.balls_played = balls_played;
        return progress;
    }

    if (active_state != AppState::Playing) {
        return progress;
    }

    if (story_runtime.active_match != StoryMatchKind::None) {
        const StoryMatchPolicyDescriptor& story_policy = story_match_policy_for_kind(story_runtime.active_match);
        progress.story_match_kind = story_runtime.active_match;
        progress.scenario_id = story_policy.scenario_id;
        progress.exit_behavior = story_policy.exit_behavior;
        progress.exit_requires_confirmation = story_policy.exit_requires_confirmation;
        progress.exit_label = story_policy.exit_label;
        progress.forfeit_unlock_balls = story_policy.forfeit_unlock_balls;
        progress.forfeit_unlock_reason = story_policy.forfeit_unlock_reason;
        if (story_policy.score_model == StoryMatchScoreModel::RallyLoop) {
            progress.session_kind = MatchSessionKind::StoryTraining;
        } else if (story_policy.score_model == StoryMatchScoreModel::BestOfGames) {
            progress.session_kind = MatchSessionKind::StoryOfficial;
        } else {
            progress.session_kind = MatchSessionKind::StoryOnboarding;
        }
        progress.balls_played = balls_played;
        return progress;
    }

    if (match_flow.mode == ActiveMatchMode::Quick) {
        progress.session_kind = MatchSessionKind::Quick;
        progress.balls_played = balls_played;
    }
    return progress;
}

MatchExitPolicy evaluate_match_exit_policy(const MatchProgress& progress) {
    MatchExitPolicy policy {};
    switch (progress.session_kind) {
        case MatchSessionKind::Quick:
            policy.has_exit_option = true;
            policy.can_exit_now = true;
            policy.requires_confirmation = false;
            policy.exit_label = "EXIT MATCH";
            policy.action = MatchExitAction::ExitQuickToSetup;
            return policy;
        case MatchSessionKind::StoryTraining:
        case MatchSessionKind::StoryOfficial:
        case MatchSessionKind::StoryOnboarding:
        case MatchSessionKind::IntroFirstMatch:
            switch (progress.exit_behavior) {
                case StoryMatchExitBehavior::StopTraining:
                    policy.has_exit_option = true;
                    policy.can_exit_now = true;
                    policy.requires_confirmation = progress.exit_requires_confirmation;
                    policy.exit_label = progress.exit_label;
                    policy.action = MatchExitAction::ExitStoryMatch;
                    policy.story_end_reason = StoryMatchEndReason::EndTraining;
                    return policy;
                case StoryMatchExitBehavior::Forfeit:
                    if (progress.balls_played < progress.forfeit_unlock_balls) {
                        policy.has_exit_option = false;
                        policy.can_exit_now = false;
                        policy.requires_confirmation = false;
                        policy.blocked_reason = progress.forfeit_unlock_reason;
                        policy.action = MatchExitAction::None;
                        return policy;
                    }
                    policy.has_exit_option = true;
                    policy.can_exit_now = true;
                    policy.requires_confirmation = progress.exit_requires_confirmation;
                    policy.exit_label = progress.exit_label;
                    policy.story_end_reason = StoryMatchEndReason::Forfeit;
                    policy.action = progress.session_kind == MatchSessionKind::IntroFirstMatch
                        ? MatchExitAction::ExitIntroContinueStory
                        : MatchExitAction::ExitStoryMatch;
                    return policy;
                case StoryMatchExitBehavior::None:
                default:
                    policy.has_exit_option = false;
                    policy.can_exit_now = false;
                    policy.requires_confirmation = false;
                    policy.action = MatchExitAction::None;
                    return policy;
            }
        case MatchSessionKind::None:
        default:
            return policy;
    }
}

}  // namespace whacker::app
