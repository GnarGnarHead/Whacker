#include "story_runtime.hpp"

#include <cassert>

#include "progression/skills.hpp"
#include "story_classification.hpp"
#include "story_runtime_invariants.hpp"
#include "story_match.hpp"
#include "story_script_catalog.hpp"
#include "story_scene.hpp"
#include "story_save_helpers.hpp"
#include "story_skill_limits.hpp"

namespace {

std::string sanitize_name_or_passthrough(
    const std::string& value,
    const whacker::app::StorySanitizeNameFn sanitize_name_fn) {
    return sanitize_name_fn != nullptr ? sanitize_name_fn(value) : value;
}

void save_career_if_possible(
    const whacker::app::StoryCareerData& career,
    const whacker::app::StorySaveCareerCallback save_career_fn,
    whacker::app::StoryHubState* story_hub_state = nullptr) {
    (void)whacker::app::persist_story_career_with_feedback(career, save_career_fn, story_hub_state);
}

void reset_onboarding_feedback_defaults(whacker::app::StoryRuntimeState& story_runtime) {
    story_runtime.onboarding_aya_feedback_available = false;
    story_runtime.onboarding_aya_feedback_from_loss = false;
    story_runtime.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Balanced;
    story_runtime.onboarding_aya_forfeited = false;
}

void apply_story_hub_defaults(whacker::app::StoryHubState& story_hub_state) {
    story_hub_state.selected_row = whacker::app::StoryHubRowOfficialMatch;
    story_hub_state.feedback_line_1.clear();
    story_hub_state.feedback_line_2.clear();
}

void apply_story_intro_invite_defaults(whacker::app::StoryIntroState& story_intro_state) {
    story_intro_state = whacker::app::StoryIntroState {};
    story_intro_state.phase = whacker::app::StoryIntroPhase::Invite;
    story_intro_state.break_kind = whacker::app::StoryIntroBreak::None;
    story_intro_state.swap_choice = 0;
    story_intro_state.player_is_right = false;
    story_intro_state.name_prompted = false;
    story_intro_state.name_accept_pending = false;
    story_intro_state.name_missing_prompt = false;
    story_intro_state.rules_hint_shown = false;
    story_intro_state.player_scored = false;
    story_intro_state.player_won = false;
    story_intro_state.player_forfeited = false;
    story_intro_state.dialogue_writing = true;
    story_intro_state.points_played = 0;
    story_intro_state.final_left_score = 0;
    story_intro_state.final_right_score = 0;
    story_intro_state.visible_chars = 0;
    story_intro_state.type_accum = 0.0f;
    story_intro_state.phase_timer = 0.0f;
    story_intro_state.typed_phase = whacker::app::StoryIntroPhase::Invite;
    story_intro_state.typed_break = whacker::app::StoryIntroBreak::None;
    story_intro_state.entered_name.clear();
    whacker::app::reset_story_intro_typewriter(story_intro_state);
}

void apply_story_intro_option_defaults(whacker::app::MatchOptions& options) {
    options.left_mode = whacker::app::PaddleMode::Human;
    options.right_mode = whacker::app::PaddleMode::AI;
    options.left_ai_style = whacker::app::AiStyle::Balanced;
    options.right_ai_style = whacker::app::AiStyle::Balanced;
    options.left_paddle_skills = whacker::progression::SkillState {.edge = 0.28f, .power = 0.28f, .spin_inject = 0.28f};
    options.right_paddle_skills = whacker::progression::SkillState {.edge = 0.28f, .power = 0.28f, .spin_inject = 0.28f};
}

void reset_match_flow_and_simulation(
    whacker::app::MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation) {
    whacker::app::reset_match_flow(match_flow);
    simulation.reset();
}

void apply_new_story_intro_runtime_defaults(whacker::app::StoryRuntimeState& story_runtime) {
    story_runtime.career_loaded = true;
    story_runtime.onboarding_step = whacker::app::StoryOnboardingStep::None;
    story_runtime.intro_style_hint = whacker::app::StoryIntroStyleHint::Balanced;
    story_runtime.intro_performance_hint = whacker::app::StoryIntroPerformanceHint::Neutral;
    story_runtime.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Balanced;
    story_runtime.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::Neutral;
    reset_onboarding_feedback_defaults(story_runtime);
    whacker::app::clear_story_runtime_scene_pending_flags(story_runtime);
    story_runtime.active_match = whacker::app::StoryMatchKind::None;
    story_runtime.active_rival_id = whacker::app::StoryRivalId::None;
    story_runtime.active_rival_style = whacker::app::AiStyle::Balanced;
    story_runtime.active_rival_skills = {};
    story_runtime.official_games_left = 0;
    story_runtime.official_games_right = 0;
    story_runtime.imagination_takeover_cue_shown = false;
    story_runtime.imagination_takeover_cue_seconds = 0.0f;
    story_runtime.career.joined_club = false;
    story_runtime.career.progression_node_id.clear();
    story_runtime.career.story_completed = false;
    story_runtime.career.tix_1967_seen = false;
    story_runtime.career.tix_1967_player_won = false;
    story_runtime.career.tix_1967_score_for = 0;
    story_runtime.career.tix_1967_score_against = 0;
    story_runtime.career.tix_midweek_scene_seen = false;
    story_runtime.career.tix_lunch_match_accepted = false;
    story_runtime.career.tix_lunch_match_declined = false;
    story_runtime.career.tix_lunch_match_completed = false;
}

void apply_post_intro_runtime_defaults(whacker::app::StoryRuntimeState& story_runtime) {
    story_runtime.onboarding_style_hint = story_runtime.intro_style_hint;
    story_runtime.onboarding_performance_hint = story_runtime.intro_performance_hint;
    reset_onboarding_feedback_defaults(story_runtime);
    whacker::app::queue_story_onboarding_scene(story_runtime, whacker::app::StoryOnboardingStep::EarlyArrivalScene);
}

void apply_intro_progression_if_enabled(
    whacker::app::StoryRuntimeState& story_runtime,
    const whacker::app::StoryIntroState& story_intro_state) {
    const whacker::app::StoryMatchPolicyDescriptor& intro_policy = whacker::app::story_intro_first_match_policy();
    if (!intro_policy.progression.xp_enabled) {
        return;
    }
    if (!story_intro_state.player_won && !intro_policy.progression.xp_on_forfeit) {
        return;
    }
    const whacker::progression::SkillUsageMetrics usage =
        whacker::progression::finalize_usage(story_intro_state.player_usage);
    whacker::progression::apply_skill_growth(story_runtime.career.player_skill_caps, usage);
    whacker::progression::apply_skill_growth(story_runtime.career.player_skills, usage);
    whacker::app::normalize_story_player_skill_progress(
        story_runtime.career.player_skills,
        story_runtime.career.player_skill_caps);
}

}  // namespace

namespace whacker::app {

bool story_hub_row_enabled(const StoryHubRow row, const StoryCareerData& career) {
    switch (row) {
        case StoryHubRowOfficialMatch:
            return career.joined_club && !career.official_completed && !career.story_completed;
        case StoryHubRowTrainingMatch:
            return career.joined_club && !career.story_completed;
        case StoryHubRowNextWeek:
            return
                career.joined_club &&
                career.official_completed &&
                !career.story_completed &&
                story_graph_has_next_node(career);
        case StoryHubRowPaddleTuning:
            return true;
        case StoryHubRowBack:
            return true;
        default:
            return false;
    }
}

void begin_new_story_intro(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchOptions& options,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    const StoryResetCareerFn reset_career_fn) {
    if (reset_career_fn != nullptr) {
        reset_career_fn(story_runtime.career);
    } else {
        story_runtime.career = StoryCareerData {};
    }
    apply_new_story_intro_runtime_defaults(story_runtime);
    copy_onboarding_runtime_to_career(story_runtime);
    reset_story_match_tracking(story_runtime);
    apply_story_hub_defaults(story_hub_state);
    apply_story_intro_invite_defaults(story_intro_state);
    apply_story_intro_option_defaults(options);
    reset_match_flow_and_simulation(match_flow, simulation);
    auto& state = simulation.mutable_state();
    state.ball.velocity.x = 0.0f;
    state.ball.velocity.y = 0.0f;
}

StoryIntroCompleteResult complete_story_intro(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    const StorySanitizeNameFn sanitize_name_fn,
    const StorySaveCareerCallback save_career_fn) {
    story_runtime.career.player_name = sanitize_name_or_passthrough(story_intro_state.entered_name, sanitize_name_fn);
    story_runtime.career.prefers_right_side = story_intro_state.player_is_right;
    story_runtime.career.joined_club = false;
    story_runtime.career.progression_node_id.clear();
    story_runtime.career.story_completed = false;
    story_runtime.intro_style_hint = classify_story_style_hint(story_intro_state.player_usage);
    const int player_score = story_intro_state.player_is_right
        ? story_intro_state.final_right_score
        : story_intro_state.final_left_score;
    const int opponent_score = story_intro_state.player_is_right
        ? story_intro_state.final_left_score
        : story_intro_state.final_right_score;
    story_runtime.intro_performance_hint =
        classify_story_performance_hint(story_intro_state.player_won, player_score, opponent_score);
    apply_intro_progression_if_enabled(story_runtime, story_intro_state);
    apply_post_intro_runtime_defaults(story_runtime);
    copy_onboarding_runtime_to_career(story_runtime);
    apply_story_hub_defaults(story_hub_state);
    const StoryMatchPolicyDescriptor& intro_policy = story_intro_first_match_policy();
    clear_authored_transition_request(authored_transition_request);
    if (story_policy_post_route_triggers_wipe(intro_policy, story_intro_state.player_forfeited)) {
        StorySceneState onboarding_target_scene {};
        begin_story_onboarding_scene(onboarding_target_scene, story_runtime);
        const TransitionArmResult arm_result = arm_authored_star_wipe_transition(
            authored_transition_request,
            Screen::StoryIntro,
            nullptr,
            Screen::StoryScene,
            &onboarding_target_scene);
        if (!arm_result.armed) {
#ifndef NDEBUG
            assert(false && "story_runtime failed to arm authored transition request.");
#endif
            clear_authored_transition_request(authored_transition_request);
        }
    }

    save_career_if_possible(story_runtime.career, save_career_fn, &story_hub_state);
    story_intro_state = StoryIntroState {};
    reset_match_flow_and_simulation(match_flow, simulation);
    return StoryIntroCompleteResult {.route = screen_route(Screen::StoryScene)};
}

}  // namespace whacker::app
