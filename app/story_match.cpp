#include "story_match.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

#include "progression/reputation.hpp"
#include "progression/skills.hpp"
#include "progression/tags.hpp"
#include "sim/math.hpp"
#include "story_script_catalog.hpp"
#include "story_save_helpers.hpp"
#include "story_skill_limits.hpp"
#include "story_classification.hpp"
#include "story_play_session.hpp"
#include "story_text.hpp"
#include "story_text_week.hpp"
#include "story_runtime_invariants.hpp"

namespace {

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

bool progression_enabled_for_result(
    const whacker::app::StoryMatchPolicyDescriptor& policy,
    const whacker::app::StoryMatchEndReason end_reason) {
    if (!policy.progression.xp_enabled) {
        return false;
    }
    if (end_reason == whacker::app::StoryMatchEndReason::Forfeit) {
        return policy.progression.xp_on_forfeit;
    }
    return true;
}

whacker::progression::SkillUsageMetrics scaled_usage_metrics(
    const whacker::progression::SkillUsageMetrics& usage,
    const float scale) {
    whacker::progression::SkillUsageMetrics scaled {};
    scaled.edge = usage.edge * scale;
    scaled.power = usage.power * scale;
    scaled.spin_inject = usage.spin_inject * scale;
    scaled.exposure = usage.exposure;
    whacker::progression::clamp_usage(scaled);
    return scaled;
}

void apply_story_progression_if_enabled(
    whacker::app::StoryRuntimeState& story_runtime,
    const whacker::app::StoryMatchPolicyDescriptor& policy,
    const whacker::progression::SkillUsageMetrics& usage,
    const whacker::app::StoryMatchEndReason end_reason) {
    if (!progression_enabled_for_result(policy, end_reason)) {
        return;
    }
    constexpr float kForfeitUsageScale = 0.50f;
    const whacker::progression::SkillUsageMetrics applied_usage =
        end_reason == whacker::app::StoryMatchEndReason::Forfeit
        ? scaled_usage_metrics(usage, kForfeitUsageScale)
        : usage;
    whacker::progression::apply_skill_growth(
        story_runtime.career.player_skill_caps,
        applied_usage);
    whacker::progression::apply_skill_growth(
        story_runtime.career.player_skills,
        applied_usage);
    whacker::app::normalize_story_player_skill_progress(
        story_runtime.career.player_skills,
        story_runtime.career.player_skill_caps);
}

void save_story_career_if_possible(
    const whacker::app::StoryCareerData& career,
    const whacker::app::StorySaveCareerFn save_career_fn,
    whacker::app::StoryHubState* story_hub_state = nullptr) {
    (void)whacker::app::persist_story_career_with_feedback(career, save_career_fn, story_hub_state);
}

}  // namespace

namespace whacker::app {

void reset_story_match_tracking(StoryRuntimeState& story_runtime) {
    story_runtime.player_usage = {};
    story_runtime.active_match_seconds = 0.0f;
    story_runtime.active_peak_lead = 0;
    story_runtime.active_peak_deficit = 0;
    story_runtime.imagination_takeover_cue_shown = false;
    story_runtime.imagination_takeover_cue_seconds = 0.0f;
}

void start_story_match(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchOptions& options,
    whacker::sim::Simulation& simulation,
    MatchFlowState& match_flow,
    std::mt19937_64& rng,
    const StoryMatchKind match_kind) {
    const StoryMatchPolicyDescriptor& policy = story_match_policy_for_kind(match_kind);
    (void)story_graph_initialize_career_node(story_runtime.career);
    const StoryRivalSpec rival = story_policy_rival_spec_for_career(policy, story_runtime.career);
    story_runtime.active_match = match_kind;
    story_runtime.active_rival_id = rival.id;
    story_runtime.active_rival_style = rival.style;
    story_runtime.active_rival_skills = rival.skills;
    story_runtime.official_games_left = 0;
    story_runtime.official_games_right = 0;
    reset_story_match_tracking(story_runtime);

    start_story_play_session(
        options,
        simulation,
        match_flow,
        rng,
        policy.session_mode,
        story_runtime.career.prefers_right_side,
        story_runtime.active_rival_style,
        story_runtime.active_rival_skills,
        story_runtime.career.player_skills);

    if (policy.narrative.update_onboarding_hints) {
        story_hub_state.feedback_line_1.clear();
        story_hub_state.feedback_line_2.clear();
    } else {
        story_text::FeedbackLines feedback = story_text::match_start_feedback(match_kind);
        const StoryGraphNodeSpec& node = story_graph_node_for_career(story_runtime.career);
        story_text::FeedbackLines override_feedback {};
        if (story_text_week::match_start_feedback(node.node_id, match_kind, override_feedback)) {
            feedback = override_feedback;
        }
        story_hub_state.feedback_line_1 = feedback.line_1;
        story_hub_state.feedback_line_2 = feedback.line_2;
    }
}

void update_story_match_tracking(
    StoryRuntimeState& story_runtime,
    const whacker::sim::SimulationConfig& config,
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const float dt) {
    if (story_runtime.active_match == StoryMatchKind::None) {
        return;
    }

    const bool player_is_right = story_runtime.career.prefers_right_side;
    story_runtime.active_match_seconds += dt;
    const int lead = player_is_right
        ? (after.right_score - after.left_score)
        : (after.left_score - after.right_score);
    story_runtime.active_peak_lead = std::max(story_runtime.active_peak_lead, lead);
    story_runtime.active_peak_deficit = std::max(story_runtime.active_peak_deficit, -lead);

    if (after.rally_hits <= before.rally_hits) {
        return;
    }

    const bool hitter_left = after.ball.velocity.x > 0.0f;
    const bool hitter_is_player = player_is_right ? !hitter_left : hitter_left;
    if (!hitter_is_player) {
        return;
    }

    const float denom = std::max(config.paddle_half_height, 1.0e-3f);
    const auto& player_paddle = player_is_right ? after.right : after.left;
    const float contact_u = clampf((after.ball.position.y - player_paddle.center_y) / denom, -1.0f, 1.0f);
    const float ball_speed = whacker::sim::speed_of(after.ball);
    whacker::progression::accumulate_contact_usage(
        story_runtime.player_usage,
        contact_u,
        player_paddle.velocity_y,
        ball_speed,
        config);
}

void finalize_story_match(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    const whacker::sim::RallyState& final_state,
    const StorySaveCareerFn save_career_fn,
    const StoryMatchEndReason end_reason) {
    if (story_runtime.active_match == StoryMatchKind::None) {
        return;
    }
    const StoryMatchPolicyDescriptor& policy = story_match_policy_for_kind(story_runtime.active_match);

    const bool player_is_right = story_runtime.career.prefers_right_side;
    int result_left_score = final_state.left_score;
    int result_right_score = final_state.right_score;
    if (policy.score_model == StoryMatchScoreModel::BestOfGames &&
        (story_runtime.official_games_left > 0 || story_runtime.official_games_right > 0)) {
        result_left_score = story_runtime.official_games_left;
        result_right_score = story_runtime.official_games_right;
    }

    const int player_score = player_is_right ? result_right_score : result_left_score;
    const int opponent_score = player_is_right ? result_left_score : result_right_score;
    const bool player_won = player_score > opponent_score;
    const bool training_tied =
        policy.score_model == StoryMatchScoreModel::RallyLoop &&
        final_state.left_score == final_state.right_score;
    const whacker::progression::SkillUsageMetrics usage =
        whacker::progression::finalize_usage(story_runtime.player_usage);

    apply_story_progression_if_enabled(story_runtime, policy, usage, end_reason);

    if (story_runtime.active_match == StoryMatchKind::Imagination1967) {
        story_runtime.career.tix_1967_seen = true;
        story_runtime.career.tix_1967_player_won = player_won;
        story_runtime.career.tix_1967_score_for = std::max(0, player_score);
        story_runtime.career.tix_1967_score_against = std::max(0, opponent_score);
        story_runtime.career.week = std::max(2, story_runtime.career.week);
        story_runtime.career.tix_midweek_scene_seen = false;
        story_runtime.career.tix_lunch_match_accepted = false;
        story_runtime.career.tix_lunch_match_declined = false;
        story_runtime.career.tix_lunch_match_completed = false;
        queue_story_onboarding_scene(story_runtime, StoryOnboardingStep::TixMidweekScene);
        story_runtime.career.joined_club = true;
        story_runtime.career.story_completed = false;
        story_runtime.career.official_completed = false;
        (void)story_graph_initialize_career_node(story_runtime.career);
        const story_text::FeedbackLines feedback =
            story_text::imagination_1967_result_feedback(player_won, player_score, opponent_score);
        story_hub_state.selected_row = StoryHubRowOfficialMatch;
        story_hub_state.feedback_line_1 = feedback.line_1;
        story_hub_state.feedback_line_2 = feedback.line_2;
    }

    if (story_runtime.active_match == StoryMatchKind::TixLunch) {
        story_runtime.career.tix_lunch_match_completed = end_reason != StoryMatchEndReason::Forfeit;
        if (story_runtime.career.tix_lunch_match_completed) {
            story_runtime.career.crew_affinity.grind_systems =
                std::max(0, story_runtime.career.crew_affinity.grind_systems) + 2;
            queue_story_onboarding_scene(story_runtime, StoryOnboardingStep::PostTixLunchScene);
        } else {
            story_hub_state.feedback_line_1 = "Lunch set logged.";
            story_hub_state.feedback_line_2 = "Tix: We'll run it back another day.";
        }
    }

    if (policy.narrative.update_onboarding_hints) {
        story_runtime.onboarding_style_hint = classify_story_style_hint(story_runtime.player_usage);
        story_runtime.onboarding_performance_hint =
            classify_story_performance_hint(player_won, player_score, opponent_score);
        story_runtime.onboarding_aya_forfeited = false;
        if (policy.narrative.update_onboarding_aya_feedback) {
            const StoryIntroStyleHint weakness_hint = classify_story_weakness_hint(usage);
            story_runtime.onboarding_aya_feedback_available = true;
            story_runtime.onboarding_aya_feedback_from_loss = !player_won;
            story_runtime.onboarding_aya_feedback_hint = player_won ? story_runtime.onboarding_style_hint : weakness_hint;
            story_runtime.onboarding_aya_forfeited = end_reason == StoryMatchEndReason::Forfeit;
        }
    }

    if (policy.counters.increment_training_used) {
        story_runtime.career.training_used = std::max(0, story_runtime.career.training_used) + 1;
        story_runtime.career.crew_affinity.grind_systems =
            std::max(0, story_runtime.career.crew_affinity.grind_systems) + 1;
    }
    if (policy.counters.increment_training_matches_played) {
        story_runtime.career.training_matches_played += 1;
    }

    if (policy.narrative.use_training_feedback) {
        const whacker::progression::TrainingBlockSummary summary {
            .training_match_count = 1,
            .training_minutes_total = story_runtime.active_match_seconds / 60.0f,
            .edge_usage_mean = usage.edge,
            .power_usage_mean = usage.power,
            .spin_inject_usage_mean = usage.spin_inject,
            .clean_contact_rate_mean = whacker::progression::clean_contact_rate(story_runtime.player_usage),
            .high_edge_contact_rate_mean = whacker::progression::high_edge_contact_rate(story_runtime.player_usage)
        };
        const std::vector<whacker::progression::TrainingTag> tags =
            whacker::progression::emit_training_tags(summary);

        if (end_reason == StoryMatchEndReason::EndTraining) {
            story_hub_state.feedback_line_1 = story_text::training_end_feedback_line_1();
        } else {
            story_hub_state.feedback_line_1 = story_text::training_result_feedback_line_1(training_tied, player_won);
        }
        const std::string primary_tag =
            tags.empty() ? std::string {} : std::string {whacker::progression::to_string(tags.front())};
        story_hub_state.feedback_line_2 = story_text::style_feedback_with_tag_line_2(usage, primary_tag);
        story_runtime.career.reactivity.last_training_tag_1 =
            tags.size() >= 1 ? static_cast<int>(tags[0]) : -1;
        story_runtime.career.reactivity.last_training_tag_2 =
            tags.size() >= 2 ? static_cast<int>(tags[1]) : -1;
    }

    if (policy.counters.mark_official_completed) {
        story_runtime.career.official_completed = true;
    }
    if (policy.counters.update_official_record) {
        if (player_won) {
            story_runtime.career.official_wins += 1;
        } else {
            story_runtime.career.official_losses += 1;
        }
    }

    if (policy.counters.update_official_forfeit_streak) {
        if (end_reason == StoryMatchEndReason::Forfeit) {
            story_runtime.career.official_forfeits_total += 1;
            story_runtime.career.official_forfeit_streak += 1;
        } else {
            story_runtime.career.official_forfeit_streak = 0;
        }
    }

    float expected = 0.0f;
    if (policy.counters.update_reputation || policy.narrative.use_official_feedback) {
        const float rival_rating = 1020.0f + (22.0f * static_cast<float>(std::max(0, story_runtime.career.week - 1)));
        expected =
            whacker::progression::expected_win_probability(story_runtime.career.reputation.rating, rival_rating);
    }

    if (policy.counters.update_reputation) {
        const whacker::progression::OfficialResultInput result_input {
            .won = player_won,
            .margin = std::abs(result_left_score - result_right_score),
            .expected_win_prob = expected
        };
        whacker::progression::apply_official_result(story_runtime.career.reputation, result_input);
    }

    if (policy.narrative.use_official_feedback) {
        const whacker::progression::OfficialMatchSummary summary {
            .won = player_won,
            .score_for = player_score,
            .score_against = opponent_score,
            .peak_deficit = story_runtime.active_peak_deficit,
            .peak_lead = story_runtime.active_peak_lead,
            .expected_win_prob = expected
        };
        const std::vector<whacker::progression::OfficialTag> tags =
            whacker::progression::emit_official_tags(summary);

        if (end_reason == StoryMatchEndReason::Forfeit) {
            story_hub_state.feedback_line_1 =
                story_text::official_forfeit_feedback_line_1(story_runtime.career.official_forfeit_streak);
        } else {
            story_hub_state.feedback_line_1 = story_text::official_result_feedback_line_1(player_won);
        }
        const std::string primary_tag =
            tags.empty() ? std::string {} : std::string {whacker::progression::to_string(tags.front())};
        story_hub_state.feedback_line_2 = story_text::official_result_feedback_line_2(
            whacker::progression::to_string(story_runtime.career.reputation.state),
            static_cast<int>(std::lround(story_runtime.career.reputation.rating)),
            player_score,
            opponent_score,
            primary_tag);
        story_runtime.career.reactivity.last_official_tag_1 =
            tags.size() >= 1 ? static_cast<int>(tags[0]) : -1;
        story_runtime.career.reactivity.last_official_tag_2 =
            tags.size() >= 2 ? static_cast<int>(tags[1]) : -1;
        story_runtime.career.reactivity.last_official_tag_3 =
            tags.size() >= 3 ? static_cast<int>(tags[2]) : -1;
    }

    save_story_career_if_possible(story_runtime.career, save_career_fn, &story_hub_state);
    story_runtime.active_match = StoryMatchKind::None;
    story_runtime.active_rival_id = StoryRivalId::None;
    story_runtime.active_rival_style = AiStyle::Balanced;
    story_runtime.active_rival_skills = {};
    story_runtime.official_games_left = 0;
    story_runtime.official_games_right = 0;
    story_runtime.imagination_takeover_cue_shown = false;
    story_runtime.imagination_takeover_cue_seconds = 0.0f;
}

void advance_story_week(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    const StorySaveCareerFn save_career_fn) {
    if (!story_runtime.career.official_completed || story_runtime.career.story_completed) {
        return;
    }
    if (!story_graph_advance_career_node(story_runtime.career)) {
        return;
    }
    story_runtime.career.reactivity.training_used_last_week = std::max(0, story_runtime.career.training_used);
    story_runtime.career.training_used = 0;
    story_runtime.career.official_completed = false;
    story_hub_state.feedback_line_1 = story_text::new_week_feedback_line_1();
    story_hub_state.feedback_line_2 = story_text::new_week_feedback_line_2();
    save_story_career_if_possible(story_runtime.career, save_career_fn, &story_hub_state);
}

}  // namespace whacker::app
