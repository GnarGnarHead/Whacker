#include "story_text.hpp"
#include "story_text_week.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>

namespace whacker::app::story_text {

namespace {

constexpr std::string_view kWeek01NodeId = "club_week_01";
constexpr std::string_view kPlayerNameToken = "{player_name}";

std::string week_one_scene_text_or(
    const whacker::app::story_text_week::SceneKey key,
    const std::string_view fallback) {
    const std::string_view authored = whacker::app::story_text_week::scene_text(kWeek01NodeId, key);
    if (!authored.empty()) {
        return std::string {authored};
    }
    return std::string {fallback};
}

std::string replace_player_name_token(std::string text, const std::string& player_name) {
    const std::size_t pos = text.find(kPlayerNameToken);
    if (pos != std::string::npos) {
        text.replace(pos, kPlayerNameToken.size(), player_name);
    }
    return text;
}

std::string week_one_player_template_or(
    const whacker::app::story_text_week::SceneKey key,
    const std::string_view fallback_template,
    const std::string& player_name) {
    return replace_player_name_token(week_one_scene_text_or(key, fallback_template), player_name);
}

}  // namespace

#ifdef WHACKER_HAS_GLFW

std::string intro_performance_line(const StoryIntroState& story_intro_state) {
    if (story_intro_state.player_forfeited) {
        return "well thanks for the game.";
    }

    const int player_score = story_intro_state.player_is_right
        ? story_intro_state.final_right_score
        : story_intro_state.final_left_score;
    const int opponent_score = story_intro_state.player_is_right
        ? story_intro_state.final_left_score
        : story_intro_state.final_right_score;
    const int margin = std::abs(player_score - opponent_score);

    if (story_intro_state.player_won) {
        if (margin >= 4) {
            return "You controlled the pace.";
        }
        if (margin <= 2) {
            return "You closed a tight game.";
        }
        return "Solid finish.";
    }

    if (margin <= 2) {
        return "That was close.";
    }
    if (player_score >= 8) {
        return "You stayed in the rallies.";
    }
    return "You kept playing through it.";
}

std::string intro_style_line(const StoryIntroState& story_intro_state) {
    if (story_intro_state.player_usage.contacts < 3) {
        return "You've got good instincts.";
    }

    const whacker::progression::SkillUsageMetrics usage =
        whacker::progression::finalize_usage(story_intro_state.player_usage);
    const float max_usage = std::max({usage.power, usage.edge, usage.spin_inject});
    const float min_usage = std::min({usage.power, usage.edge, usage.spin_inject});
    if ((max_usage - min_usage) < 0.08f) {
        return "You mix shots well.";
    }

    if (usage.power >= usage.edge && usage.power >= usage.spin_inject) {
        return "Those center shots had weight.";
    }
    if (usage.edge >= usage.power && usage.edge >= usage.spin_inject) {
        return "Those were some sharp angles.";
    }
    return "Those spin shots were crazy.";
}

std::string intro_header_line(const std::string& rival_name) {
    return rival_name + " / After school gym";
}

std::string intro_invite_line_1() {
    return "Hey, hey you... want to play a game?";
}

std::string intro_invite_line_2() {
    return "Gym's quiet. Want to hit for a bit?";
}

std::string intro_invite_line_3() {
    return "Press Enter";
}

std::string intro_swap_sides_line_1() {
    return "Want to stay P1 or switch to P2?";
}

std::string intro_swap_sides_line_2() {
    return "Left/Right choose  Enter confirm";
}

std::string intro_swap_option_stay_left() {
    return "Stay left (P1)";
}

std::string intro_swap_option_swap_right() {
    return "Swap right (P2)";
}

std::string intro_controls_line_1() {
    return "No stress. Keys are:";
}

std::string intro_controls_line_2(const std::string& up_key_name, const std::string& down_key_name) {
    return "Use " + up_key_name + " (UP) / " + down_key_name + " (DOWN).";
}

std::string intro_controls_line_3() {
    return "Press Enter";
}

std::string intro_rules_line_1() {
    return "Rules: first to 11. Win by 2.";
}

std::string intro_rules_line_2() {
    return "Serve changes every 2. At 10-10, every point.";
}

std::string intro_rules_line_3() {
    return "Press Enter";
}

std::string intro_ready_next_ball_line_1() {
    return "Ready for the next ball?";
}

std::string intro_ready_next_ball_line_2() {
    return "Press Enter";
}

std::string intro_name_confirm_line_1() {
    return "Lock this name in?";
}

std::string intro_name_confirm_line_2() {
    return "Enter accept  Backspace edit";
}

std::string intro_name_prompt_line_1() {
    return "So what's your name?";
}

std::string intro_name_prompt_line_2(const bool missing_prompt) {
    return missing_prompt
        ? "Enter a name first."
        : "Type, Backspace delete, Enter ready";
}

std::string intro_name_placeholder_line_3() {
    return "_";
}

std::string intro_rival_intro_line_1(const std::string& player_name, const std::string& rival_name) {
    return "Hey, nice to meet you, " + player_name + ". I'm " + rival_name + ", by the way.";
}

std::string intro_rival_intro_line_2(
    const int final_left_score,
    const int final_right_score,
    const std::string& performance_line,
    const bool player_forfeited) {
    if (player_forfeited) {
        return performance_line;
    }
    return "FINAL " + std::to_string(final_left_score) + "-" +
        std::to_string(final_right_score) + "  " + performance_line;
}

std::string intro_rival_intro_line_3(const std::string& style_line) {
    return style_line + " We have a school club that meets on Fridays, if you're interested. Press Enter.";
}

#endif  // WHACKER_HAS_GLFW

std::string onboarding_early_arrival_header() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingEarlyArrivalHeader,
        "White Lions / Early arrival");
}

std::string onboarding_club_floor_header() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingClubFloorHeader,
        "White Lions / Club floor");
}

std::string onboarding_coach_reyes_header() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingCoachReyesHeader,
        "White Lions / Coach Reyes");
}

std::string at_home_youtube_header() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::AtHomeYoutubeHeader,
        "Bedroom / Late night");
}

std::string tix_midweek_scene_header() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::TixMidweekSceneHeader,
        "White Lions / Midweek");
}

std::string onboarding_aya_early_arrival_line_1() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingAyaEarlyArrivalLine1,
        "AYA: You're early.");
}

std::string onboarding_aya_early_arrival_line_2(const std::string& player_name) {
    return week_one_player_template_or(
        whacker::app::story_text_week::SceneKey::OnboardingAyaEarlyArrivalLine2Template,
        "AYA: I'm Aya. You're {player_name}, right?",
        player_name);
}

std::string onboarding_aya_early_arrival_line_3() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingAyaEarlyArrivalLine3,
        "AYA: Kai mentioned you. Glad you made it.");
}

std::string onboarding_aya_early_arrival_line_4() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingAyaEarlyArrivalLine4,
        "AYA: We've got a few quiet minutes. Want a short warm-up?");
}

std::string onboarding_aya_intro_to_coach_line(const std::string& player_name) {
    return week_one_player_template_or(
        whacker::app::story_text_week::SceneKey::OnboardingAyaIntroToCoachTemplate,
        "AYA: Coach, this is {player_name}. We just hit a warm-up.",
        player_name);
}

std::string onboarding_coach_intro_player_line(const std::string& player_name) {
    return week_one_player_template_or(
        whacker::app::story_text_week::SceneKey::OnboardingCoachIntroPlayerTemplate,
        "COACH REYES: Reyes. Good to meet you, {player_name}.",
        player_name);
}

std::string onboarding_coach_welcome_line() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingCoachWelcomeLine,
        "COACH REYES: Let's see what you can do.");
}

std::string onboarding_coach_assign_benji_line() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingCoachAssignBenjiLine,
        "COACH REYES: Benji. Table two.");
}

std::string onboarding_coach_benji_spin_warning_line() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingCoachBenjiSpinWarningLine,
        "COACH REYES: Careful, he puts spin on everything.");
}

std::string onboarding_coach_entry_retry_line() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingCoachEntryRetryLine,
        "COACH REYES: You'll need to play a full match so I can assess your skill level.");
}

std::string onboarding_coach_post_entry_compliment_line(
    const StoryIntroPerformanceHint performance_hint,
    const StoryIntroStyleHint style_hint) {
    if (performance_hint == StoryIntroPerformanceHint::CloseLoss) {
        return "COACH REYES: Good response work. You stayed in a tight game.";
    }

    switch (style_hint) {
        case StoryIntroStyleHint::Power:
            return "COACH REYES: Good center contact. You generated real pace.";
        case StoryIntroStyleHint::Technical:
            return "COACH REYES: Good angle management. Your contact choices held up.";
        case StoryIntroStyleHint::Spin:
            return "COACH REYES: Good spin management. You made their reads hard.";
        case StoryIntroStyleHint::Balanced:
        default:
            return "COACH REYES: Good point building. Your decisions stayed stable.";
    }
}

std::string onboarding_coach_day_end_line() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingCoachDayEndLine,
        "COACH REYES: That's enough for today.");
}

std::string onboarding_coach_training_open_line() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingCoachTrainingOpenLine,
        "COACH REYES: Training tables are open any time. Play as many practice matches as you want.");
}

std::string onboarding_coach_training_reps_line() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingCoachTrainingRepsLine,
        "COACH REYES: Use those reps to sharpen your game.");
}

std::string onboarding_tix_post_day_line_1() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingTixPostDayLine1,
        "TIX: Tix. Technical table.");
}

std::string onboarding_tix_post_day_line_2() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingTixPostDayLine2,
        "TIX: You load your shoulder early on crosscourt returns. Benji read it twice. Easy fix.");
}

std::string onboarding_tix_post_day_line_3() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingTixPostDayLine3,
        "TIX: You know the '67 World Championships?");
}

std::string onboarding_tix_post_day_line_4() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingTixPostDayLine4,
        "TIX: Look it up tonight. Full final, not highlights.");
}

std::string onboarding_tix_post_day_line_5() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingTixPostDayLine5,
        "TIX: If you want to understand this game, start there.");
}

std::string at_home_youtube_line_1() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::AtHomeYoutubeLine1,
        "SEARCH: \"1967 world championships table tennis final\"");
}

std::string at_home_youtube_line_2() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::AtHomeYoutubeLine2,
        "IMAGINE: The film starts. Press Enter to step into point five.");
}

std::string imagination_takeover_cue_line() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::ImaginationTakeoverCueLine,
        "I could see myself playing in the match.");
}

std::string tix_midweek_scene_line_1() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::TixMidweekSceneLine1,
        "TIX: I can tell you watched it. Good.");
}

std::string tix_midweek_scene_line_2() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::TixMidweekSceneLine2,
        "TIX: Point five: when he waits an extra beat. You saw it, right?");
}

std::string tix_midweek_scene_line_3() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::TixMidweekSceneLine3,
        "TIX: That's not hesitation. It's a trap. He waits until you move.");
}

std::string tix_midweek_scene_line_4() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::TixMidweekSceneLine4,
        "TIX: Then he takes the table from you.");
}

std::string tix_midweek_scene_line_5() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::TixMidweekSceneLine5,
        "TIX: Let's get a game in before the next class. One game?");
}

std::string tix_post_lunch_line_1() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::TixPostLunchLine1,
        "TIX: Thanks for the game.");
}

std::string tix_post_lunch_line_2() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::TixPostLunchLine2,
        "TIX: Don't be late.");
}

std::string onboarding_aya_guidance_after_win_line(const StoryIntroStyleHint strength_hint) {
    switch (strength_hint) {
        case StoryIntroStyleHint::Power:
            return "AYA: Thanks for the game. Your center contact felt strong. Keep one calm safety angle ready.";
        case StoryIntroStyleHint::Technical:
            return "AYA: Thanks for the game. Your angle feel is good. When it gets tight, center can steady you.";
        case StoryIntroStyleHint::Spin:
            return "AYA: Thanks for the game. Your spin reads were good. Starting simple first can make the next ball cleaner.";
        case StoryIntroStyleHint::Balanced:
        default:
            return "AYA: Thanks for the game. You adapted well. One reliable finish pattern will help under pressure.";
    }
}

std::string onboarding_aya_guidance_after_loss_line(const StoryIntroStyleHint weakness_hint) {
    switch (weakness_hint) {
        case StoryIntroStyleHint::Power:
            return "AYA: Thanks for the game. You stayed with it. We can build center contact first if you want.";
        case StoryIntroStyleHint::Technical:
            return "AYA: Thanks for the game. You were close on the edges. Earlier reads will come quickly.";
        case StoryIntroStyleHint::Spin:
            return "AYA: Thanks for the game. You were close on spin. Starting your read a little earlier should help.";
        case StoryIntroStyleHint::Balanced:
        default:
            return "AYA: Thanks for the game. That was a lot of hard choices. One simple pattern will make next one easier.";
    }
}

std::string post_forfeit_scene_header() {
    return "White Lions / After match";
}

std::string post_forfeit_scene_line_1(const int forfeit_streak) {
    if (forfeit_streak >= 3) {
        return "COACH REYES: You're not in trouble. We're going to fix this, together.";
    }
    if (forfeit_streak == 2) {
        return "COACH REYES: Second walk-off. Talk to me before it gets that far.";
    }
    return "COACH REYES: If you need to step out, step out. No shame in that.";
}

std::string post_forfeit_scene_line_2(const int forfeit_streak) {
    if (forfeit_streak >= 2) {
        return "AYA: If today was heavy, say it early. We can adjust the plan.";
    }
    return "AYA: You're still part of this. Catch your breath, then come back.";
}

std::string post_forfeit_scene_line_3(const int forfeit_streak) {
    if (forfeit_streak >= 3) {
        return "COACH REYES: Next match, stay in. One point at a time is enough.";
    }
    if (forfeit_streak == 2) {
        return "TIX: Finishing points gives us signal. Walking off gives us noise.";
    }
    return "COACH REYES: Next time, give me a word and we'll reset clean.";
}

std::string onboarding_aya_forfeit_feedback_line() {
    return week_one_scene_text_or(
        whacker::app::story_text_week::SceneKey::OnboardingAyaForfeitFeedbackLine,
        "Are you feeling okay? No worries we can play someother time.");
}

}  // namespace whacker::app::story_text
