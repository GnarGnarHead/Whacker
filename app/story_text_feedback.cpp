#include "story_text.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace whacker::app::story_text {

FeedbackLines match_start_feedback(const StoryMatchKind match_kind) {
    switch (match_kind) {
        case StoryMatchKind::Training:
            return FeedbackLines {
                .line_1 = "Training set started.",
                .line_2 = "Unlimited reps. Esc pauses; stop training from pause menu."
            };
        case StoryMatchKind::Official:
            return FeedbackLines {
                .line_1 = "Next Match started.",
                .line_2 = "Best of 5 games. Game to 11, win by 2."
            };
        case StoryMatchKind::Imagination1967:
            return FeedbackLines {
                .line_1 = "Imagination set started.",
                .line_2 = "Watch four points, then take over."
            };
        case StoryMatchKind::TixLunch:
            return FeedbackLines {
                .line_1 = "Lunch set: Tix.",
                .line_2 = "Single game. No excuses."
            };
        case StoryMatchKind::OnboardingAyaFriendly:
        case StoryMatchKind::OnboardingEntry:
        case StoryMatchKind::None:
        default:
            return FeedbackLines {};
    }
}

std::string style_feedback_line(const whacker::progression::SkillUsageMetrics& usage) {
    if (usage.power >= usage.edge && usage.power >= usage.spin_inject) {
        return "Teammate: Your center strikes are getting heavier.";
    }
    if (usage.edge >= usage.power && usage.edge >= usage.spin_inject) {
        return "Teammate: Your edge angles look sharper.";
    }
    return "Teammate: Your spin timing is improving.";
}

std::string style_feedback_with_tag_line_2(
    const whacker::progression::SkillUsageMetrics& usage,
    const std::string& primary_tag) {
    std::string line = style_feedback_line(usage);
    if (!primary_tag.empty()) {
        line += " Tag: ";
        line += primary_tag;
    }
    return line;
}

std::string training_result_feedback_line_1(const bool training_tied, const bool player_won) {
    if (training_tied) {
        return "Training session logged.";
    }
    return player_won ? "Training win. Good reps." : "Training loss. Useful reps.";
}

std::string training_end_feedback_line_1() {
    return "Training ended. Good reps.";
}

std::string official_result_feedback_line_1(const bool player_won) {
    return player_won ? "Next Match won. Team is buzzing." : "Next Match lost. Team stays with you.";
}

std::string official_forfeit_feedback_line_1(const int forfeit_streak) {
    if (forfeit_streak >= 3) {
        return "Next Match forfeited. Team is concerned, not done with you.";
    }
    if (forfeit_streak == 2) {
        return "Next Match forfeited. Team wants a reset plan with you.";
    }
    return "Next Match forfeited. Team checked in and kept the door open.";
}

std::string official_result_feedback_line_2(
    const std::string& reputation_state,
    const int rounded_rating,
    const int score_for,
    const int score_against,
    const std::string& primary_tag) {
    std::string line = "Reputation ";
    line += reputation_state;
    line += "  Rating ";
    line += std::to_string(rounded_rating);
    line += "  Games ";
    line += std::to_string(score_for);
    line += "-";
    line += std::to_string(score_against);
    if (!primary_tag.empty()) {
        line += "  Tag ";
        line += primary_tag;
    }
    return line;
}

std::string new_week_feedback_line_1() {
    return "Story advanced.";
}

std::string new_week_feedback_line_2() {
    return "New chapter unlocked.";
}

FeedbackLines onboarding_complete_feedback() {
    return FeedbackLines {
        .line_1 = "Coach Reyes: Good work today.",
        .line_2 = "Feel free to train anytime."
    };
}

FeedbackLines imagination_1967_result_feedback(
    const bool player_won,
    const int score_for,
    const int score_against) {
    return FeedbackLines {
        .line_1 = player_won
            ? "Imagination set complete. You held the pace."
            : "Imagination set complete. Good reads under pressure.",
        .line_2 =
            "1967 log saved for Tix: " + std::to_string(score_for) + "-" + std::to_string(score_against),
    };
}

std::string forfeit_recorded_line() {
    return "Forfeit recorded.";
}

std::string continue_failed_feedback_line_1() {
    return "Continue failed.";
}

std::string career_loaded_feedback_line_1() {
    return "Career loaded.";
}

std::string career_loaded_feedback_line_2(const int week) {
    return "Week " + std::to_string(std::max(1, week)) + " ready.";
}

}  // namespace whacker::app::story_text
