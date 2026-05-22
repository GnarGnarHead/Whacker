#include "match_end_flow.hpp"

#include <cassert>
#include <string>

#include "story_script_catalog.hpp"
#include "story_save_helpers.hpp"
#include "story_runtime_invariants.hpp"
#include "story_scene.hpp"
#include "story_text.hpp"
#include "story_transition_materialization.hpp"

namespace {

void persist_story_progress(
    whacker::app::StoryRuntimeState& story_runtime,
    whacker::app::StoryHubState& story_hub_state,
    const whacker::app::StorySaveCareerCallback save_career_fn) {
    whacker::app::copy_onboarding_runtime_to_career(story_runtime);
    (void)whacker::app::persist_story_career_with_feedback(story_runtime.career, save_career_fn, &story_hub_state);
}

void maybe_arm_story_post_route_transition(
    whacker::app::RuntimeAuthoredTransitionRequest& authored_transition_request,
    const bool trigger_wipe,
    const whacker::app::Screen from_screen,
    const whacker::app::StorySceneState& from_story_scene_state,
    const whacker::app::Screen to_screen,
    const whacker::app::StorySceneState& to_story_scene_state) {
    if (!trigger_wipe) {
        return;
    }
    const whacker::app::StorySceneState* from_scene_ptr =
        from_screen == whacker::app::Screen::StoryScene ? &from_story_scene_state : nullptr;
    const whacker::app::StorySceneState* to_scene_ptr =
        to_screen == whacker::app::Screen::StoryScene ? &to_story_scene_state : nullptr;
    const whacker::app::TransitionArmResult arm_result = whacker::app::arm_authored_star_wipe_transition(
        authored_transition_request,
        from_screen,
        from_scene_ptr,
        to_screen,
        to_scene_ptr);
    if (!arm_result.armed) {
#ifndef NDEBUG
        assert(false && "match_end_flow failed to arm authored transition request.");
#endif
        whacker::app::clear_authored_transition_request(authored_transition_request);
    }
}

whacker::app::Screen apply_story_post_route(
    whacker::app::StoryRuntimeState& story_runtime,
    const whacker::app::StoryMatchPolicyDescriptor& policy,
    const bool forfeiting,
    const whacker::app::Screen from_screen,
    const whacker::app::StorySceneState& from_story_scene_state,
    whacker::app::StorySceneState& story_scene_state,
    whacker::app::RuntimeAuthoredTransitionRequest& authored_transition_request) {
    const bool trigger_wipe = whacker::app::story_policy_post_route_triggers_wipe(policy, forfeiting);
    const whacker::app::StoryMatchPostRoute route =
        forfeiting ? policy.post_route_forfeit : policy.post_route_completed;
    whacker::app::Screen target_screen = whacker::app::Screen::StoryHub;
    switch (route) {
        case whacker::app::StoryMatchPostRoute::OnboardingClubIntroScene:
            whacker::app::queue_story_onboarding_scene(story_runtime, whacker::app::StoryOnboardingStep::ClubIntroScene);
            target_screen = whacker::app::Screen::StoryScene;
            break;
        case whacker::app::StoryMatchPostRoute::OnboardingCoachBriefScene:
            whacker::app::queue_story_onboarding_scene(story_runtime, whacker::app::StoryOnboardingStep::CoachBriefScene);
            target_screen = whacker::app::Screen::StoryScene;
            break;
        case whacker::app::StoryMatchPostRoute::OnboardingEntryRetryScene: {
            const whacker::app::StoryOnboardingStep retry_step =
                policy.retry_step_if_forfeit == whacker::app::StoryOnboardingStep::None
                ? whacker::app::StoryOnboardingStep::EntryRetryScene
                : policy.retry_step_if_forfeit;
            whacker::app::queue_story_onboarding_scene(story_runtime, retry_step);
            target_screen = whacker::app::Screen::StoryScene;
            break;
        }
        case whacker::app::StoryMatchPostRoute::PostForfeitSupportScene:
            whacker::app::queue_story_post_forfeit_scene(story_runtime);
            target_screen = whacker::app::Screen::StoryScene;
            break;
        case whacker::app::StoryMatchPostRoute::StoryScene:
            target_screen = whacker::app::Screen::StoryScene;
            break;
        case whacker::app::StoryMatchPostRoute::StoryHub:
        default:
            whacker::app::clear_story_runtime_scene_pending_flags(story_runtime);
            target_screen = whacker::app::Screen::StoryHub;
            break;
    }
    const whacker::app::StorySceneState to_story_scene_state = whacker::app::materialize_story_scene_transition_target(
        story_scene_state,
        story_runtime,
        target_screen);
    maybe_arm_story_post_route_transition(
        authored_transition_request,
        trigger_wipe,
        from_screen,
        from_story_scene_state,
        target_screen,
        to_story_scene_state);
    return target_screen;
}

}  // namespace

namespace whacker::app {

MatchEndFlowResult end_active_or_quick_match(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    StorySceneState& story_scene_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    const Screen from_screen,
    const StoryMatchEndReason end_reason,
    const int story_official_games_to_win,
    const StorySaveCareerCallback save_career_fn) {
    clear_authored_transition_request(authored_transition_request);
    if (story_runtime.active_match != StoryMatchKind::None) {
        const StorySceneState from_story_scene_state = story_scene_state;
        const StoryMatchKind completed_kind = story_runtime.active_match;
        const StoryMatchPolicyDescriptor& policy = story_match_policy_for_kind(completed_kind);
        whacker::sim::RallyState terminal_state = simulation.state();
        const bool ending_training = end_reason == StoryMatchEndReason::EndTraining;
        const bool forfeiting = end_reason == StoryMatchEndReason::Forfeit;
        const bool player_is_right = story_runtime.career.prefers_right_side;

        if (policy.score_model == StoryMatchScoreModel::BestOfGames && forfeiting) {
            if (player_is_right) {
                story_runtime.official_games_left = story_official_games_to_win;
                story_runtime.official_games_right = 0;
            } else {
                story_runtime.official_games_left = 0;
                story_runtime.official_games_right = story_official_games_to_win;
            }
        } else if (forfeiting && !ending_training) {
            if (player_is_right) {
                terminal_state.left_score = 1;
                terminal_state.right_score = 0;
            } else {
                terminal_state.left_score = 0;
                terminal_state.right_score = 1;
            }
        }

        finalize_story_match(story_runtime, story_hub_state, terminal_state, nullptr, end_reason);
        if (forfeiting && !ending_training && policy.prepend_forfeit_feedback) {
            const std::string forfeit_text = story_text::forfeit_recorded_line();
            if (story_hub_state.feedback_line_1.empty()) {
                story_hub_state.feedback_line_1 = forfeit_text;
            } else {
                story_hub_state.feedback_line_1 = forfeit_text + " " + story_hub_state.feedback_line_1;
            }
        }
        reset_match_flow(match_flow);
        simulation.reset();
        const Screen route = apply_story_post_route(
            story_runtime,
            policy,
            forfeiting,
            from_screen,
            from_story_scene_state,
            story_scene_state,
            authored_transition_request);
        persist_story_progress(story_runtime, story_hub_state, save_career_fn);
        return MatchEndFlowResult {.route = route};
    }

    const ActiveMatchMode previous_mode = match_flow.mode;
    reset_match_flow(match_flow);
    simulation.reset();
    return MatchEndFlowResult {
        .route = previous_mode == ActiveMatchMode::Quick ? Screen::QuickMatchSetup : Screen::MainMenu,
    };
}

}  // namespace whacker::app
