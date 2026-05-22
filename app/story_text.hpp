#pragma once

#include <string>

#include "progression/skills.hpp"
#include "story_state.hpp"
#include "story_intro.hpp"

namespace whacker::app::story_text {

struct FeedbackLines {
    std::string line_1;
    std::string line_2;
};

std::string intro_performance_line(const StoryIntroState& story_intro_state);
std::string intro_style_line(const StoryIntroState& story_intro_state);

std::string intro_header_line(const std::string& rival_name);
std::string intro_invite_line_1();
std::string intro_invite_line_2();
std::string intro_invite_line_3();

std::string intro_swap_sides_line_1();
std::string intro_swap_sides_line_2();
std::string intro_swap_option_stay_left();
std::string intro_swap_option_swap_right();

std::string intro_controls_line_1();
std::string intro_controls_line_2(const std::string& up_key_name, const std::string& down_key_name);
std::string intro_controls_line_3();

std::string intro_rules_line_1();
std::string intro_rules_line_2();
std::string intro_rules_line_3();

std::string intro_ready_next_ball_line_1();
std::string intro_ready_next_ball_line_2();

std::string intro_name_confirm_line_1();
std::string intro_name_confirm_line_2();
std::string intro_name_prompt_line_1();
std::string intro_name_prompt_line_2(bool missing_prompt);
std::string intro_name_placeholder_line_3();

std::string intro_rival_intro_line_1(const std::string& player_name, const std::string& rival_name);
std::string intro_rival_intro_line_2(
    int final_left_score,
    int final_right_score,
    const std::string& performance_line,
    bool player_forfeited);
std::string intro_rival_intro_line_3(const std::string& style_line);

std::string onboarding_early_arrival_header();
std::string onboarding_club_floor_header();
std::string onboarding_coach_reyes_header();
std::string at_home_youtube_header();
std::string tix_midweek_scene_header();

std::string onboarding_aya_early_arrival_line_1();
std::string onboarding_aya_early_arrival_line_2(const std::string& player_name);
std::string onboarding_aya_early_arrival_line_3();
std::string onboarding_aya_early_arrival_line_4();

std::string onboarding_aya_intro_to_coach_line(const std::string& player_name);
std::string onboarding_coach_intro_player_line(const std::string& player_name);
std::string onboarding_coach_welcome_line();
std::string onboarding_coach_assign_benji_line();
std::string onboarding_coach_benji_spin_warning_line();
std::string onboarding_coach_entry_retry_line();

std::string onboarding_coach_post_entry_compliment_line(
    StoryIntroPerformanceHint performance_hint,
    StoryIntroStyleHint style_hint);
std::string onboarding_coach_day_end_line();
std::string onboarding_coach_training_open_line();
std::string onboarding_coach_training_reps_line();
std::string onboarding_tix_post_day_line_1();
std::string onboarding_tix_post_day_line_2();
std::string onboarding_tix_post_day_line_3();
std::string onboarding_tix_post_day_line_4();
std::string onboarding_tix_post_day_line_5();
std::string at_home_youtube_line_1();
std::string at_home_youtube_line_2();
std::string imagination_takeover_cue_line();
std::string tix_midweek_scene_line_1();
std::string tix_midweek_scene_line_2();
std::string tix_midweek_scene_line_3();
std::string tix_midweek_scene_line_4();
std::string tix_midweek_scene_line_5();
std::string tix_post_lunch_line_1();
std::string tix_post_lunch_line_2();

std::string onboarding_aya_guidance_after_win_line(StoryIntroStyleHint strength_hint);
std::string onboarding_aya_guidance_after_loss_line(StoryIntroStyleHint weakness_hint);
std::string post_forfeit_scene_header();
std::string post_forfeit_scene_line_1(int forfeit_streak);
std::string post_forfeit_scene_line_2(int forfeit_streak);
std::string post_forfeit_scene_line_3(int forfeit_streak);

FeedbackLines match_start_feedback(StoryMatchKind match_kind);

std::string style_feedback_line(const whacker::progression::SkillUsageMetrics& usage);
std::string style_feedback_with_tag_line_2(
    const whacker::progression::SkillUsageMetrics& usage,
    const std::string& primary_tag);

std::string training_result_feedback_line_1(bool training_tied, bool player_won);
std::string training_end_feedback_line_1();
std::string official_result_feedback_line_1(bool player_won);
std::string official_forfeit_feedback_line_1(int forfeit_streak);
std::string official_result_feedback_line_2(
    const std::string& reputation_state,
    int rounded_rating,
    int score_for,
    int score_against,
    const std::string& primary_tag);

std::string new_week_feedback_line_1();
std::string new_week_feedback_line_2();

FeedbackLines onboarding_complete_feedback();
FeedbackLines imagination_1967_result_feedback(bool player_won, int score_for, int score_against);
std::string onboarding_aya_forfeit_feedback_line();
std::string forfeit_recorded_line();

std::string continue_failed_feedback_line_1();
std::string career_loaded_feedback_line_1();
std::string career_loaded_feedback_line_2(int week);

}  // namespace whacker::app::story_text
