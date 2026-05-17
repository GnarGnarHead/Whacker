#pragma once

#include <cstdint>
#include <string_view>

#include "story_state.hpp"
#include "story_text.hpp"

namespace whacker::app::story_text_week {

enum class SceneKey : std::uint8_t {
    OnboardingEarlyArrivalHeader = 0,
    OnboardingClubFloorHeader = 1,
    OnboardingCoachReyesHeader = 2,
    AtHomeYoutubeHeader = 3,
    TixMidweekSceneHeader = 4,

    OnboardingAyaEarlyArrivalLine1 = 5,
    OnboardingAyaEarlyArrivalLine2Template = 6,
    OnboardingAyaEarlyArrivalLine3 = 7,
    OnboardingAyaEarlyArrivalLine4 = 8,
    OnboardingAyaIntroToCoachTemplate = 9,
    OnboardingCoachIntroPlayerTemplate = 10,
    OnboardingCoachWelcomeLine = 11,
    OnboardingCoachAssignBenjiLine = 12,
    OnboardingCoachBenjiSpinWarningLine = 13,
    OnboardingCoachEntryRetryLine = 14,
    OnboardingCoachDayEndLine = 15,
    OnboardingCoachTrainingOpenLine = 16,
    OnboardingCoachTrainingRepsLine = 17,

    OnboardingTixPostDayLine1 = 18,
    OnboardingTixPostDayLine2 = 19,
    OnboardingTixPostDayLine3 = 20,
    OnboardingTixPostDayLine4 = 21,
    OnboardingTixPostDayLine5 = 22,

    AtHomeYoutubeLine1 = 23,
    AtHomeYoutubeLine2 = 24,
    ImaginationTakeoverCueLine = 25,

    TixMidweekSceneLine1 = 26,
    TixMidweekSceneLine2 = 27,
    TixMidweekSceneLine3 = 28,
    TixMidweekSceneLine4 = 29,
    TixMidweekSceneLine5 = 30,

    OnboardingAyaForfeitFeedbackLine = 31,

    TixPostLunchLine1 = 32,
    TixPostLunchLine2 = 33,
};

std::string_view scene_text(std::string_view node_id, SceneKey key);

// Returns true only when the node has an authored override for this match-start feedback.
bool match_start_feedback(
    std::string_view node_id,
    StoryMatchKind match_kind,
    story_text::FeedbackLines& out_feedback);

}  // namespace whacker::app::story_text_week
