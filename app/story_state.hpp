#pragma once

#include <cstdint>
#include <string>

#include "app_types.hpp"
#include "progression/reputation.hpp"
#include "progression/skills.hpp"
#include "story_skill_limits.hpp"

namespace whacker::app {

enum class StoryMatchKind : std::uint8_t {
    None = 0,
    Training = 1,
    Official = 2,
    OnboardingAyaFriendly = 3,
    OnboardingEntry = 4,
    Imagination1967 = 5,
    TixLunch = 6,
};

enum class StoryOnboardingStep : std::uint8_t {
    None = 0,
    EarlyArrivalScene = 1,
    AyaFriendlyMatch = 2,
    ClubIntroScene = 3,
    EntryBenchmarkMatch = 4,
    CoachBriefScene = 5,
    EntryRetryScene = 6,
    Complete = 7,
    AtHomeYoutubeScene = 8,
    Imagination1967Match = 9,
    TixMidweekScene = 10,
    PostTixLunchScene = 11,
};

enum class StoryIntroStyleHint : std::uint8_t {
    Balanced = 0,
    Power = 1,
    Technical = 2,
    Spin = 3
};

enum class StoryIntroPerformanceHint : std::uint8_t {
    Neutral = 0,
    BigWin = 1,
    CloseLoss = 2
};

enum class StoryRivalId : std::uint8_t {
    None = 0,
    Kai = 1,
    Aya = 2,
    Juno = 3,
    Rook = 4,
    Mira = 5,
    Vex = 6,
    Nova = 7,
    Benji = 8,
    Tix = 9,
    Issa = 10,
    Jolo = 11,
};

struct StoryHubState {
    int selected_row = 0;
    std::string feedback_line_1;
    std::string feedback_line_2;
};

enum class StoryCrewId : std::uint8_t {
    GrindSystems = 0,
    HeartSocial = 1,
    ChaosTalent = 2,
};

struct StoryCrewAffinity {
    int grind_systems = 0;
    int heart_social = 0;
    int chaos_talent = 0;
};

struct StoryReactivityTelemetry {
    int training_used_last_week = 0;

    int last_training_tag_1 = -1;
    int last_training_tag_2 = -1;
    int last_official_tag_1 = -1;
    int last_official_tag_2 = -1;
    int last_official_tag_3 = -1;
};

struct StoryCareerData {
    int version = 1;
    int week = 1;
    std::string progression_node_id;
    bool story_completed = false;
    std::string player_name = "PLAYER";
    bool prefers_right_side = false;
    bool joined_club = false;
    int training_used = 0;
    bool official_completed = false;

    whacker::progression::SkillState player_skills = kStoryPlayerStarterSkills;
    whacker::progression::SkillState player_skill_caps = kStoryPlayerStarterSkills;
    whacker::progression::ReputationTracker reputation {};

    int official_wins = 0;
    int official_losses = 0;
    int training_matches_played = 0;
    int official_forfeits_total = 0;
    int official_forfeit_streak = 0;

    StoryCrewAffinity crew_affinity {};
    StoryReactivityTelemetry reactivity {};

    StoryOnboardingStep onboarding_step = StoryOnboardingStep::None;
    StoryIntroStyleHint onboarding_style_hint = StoryIntroStyleHint::Balanced;
    StoryIntroPerformanceHint onboarding_performance_hint = StoryIntroPerformanceHint::Neutral;
    bool onboarding_aya_feedback_available = false;
    bool onboarding_aya_feedback_from_loss = false;
    StoryIntroStyleHint onboarding_aya_feedback_hint = StoryIntroStyleHint::Balanced;
    bool onboarding_aya_forfeited = false;
    bool tix_1967_seen = false;
    bool tix_1967_player_won = false;
    int tix_1967_score_for = 0;
    int tix_1967_score_against = 0;
    bool tix_midweek_scene_seen = false;
    bool tix_lunch_match_accepted = false;
    bool tix_lunch_match_declined = false;
    bool tix_lunch_match_completed = false;
};

struct StoryRuntimeState {
    StoryCareerData career {};
    bool career_loaded = false;
    bool onboarding_scene_pending = false;
    StoryOnboardingStep onboarding_step = StoryOnboardingStep::None;
    StoryIntroStyleHint intro_style_hint = StoryIntroStyleHint::Balanced;
    StoryIntroPerformanceHint intro_performance_hint = StoryIntroPerformanceHint::Neutral;
    StoryIntroStyleHint onboarding_style_hint = StoryIntroStyleHint::Balanced;
    StoryIntroPerformanceHint onboarding_performance_hint = StoryIntroPerformanceHint::Neutral;
    bool onboarding_aya_feedback_available = false;
    bool onboarding_aya_feedback_from_loss = false;
    StoryIntroStyleHint onboarding_aya_feedback_hint = StoryIntroStyleHint::Balanced;
    bool onboarding_aya_forfeited = false;
    StoryMatchKind active_match = StoryMatchKind::None;
    StoryRivalId active_rival_id = StoryRivalId::None;
    AiStyle active_rival_style = AiStyle::Balanced;
    whacker::progression::SkillState active_rival_skills {.edge = 0.0f, .power = 0.0f, .spin_inject = 0.0f};
    whacker::progression::SkillUsageAccumulator player_usage {};
    float active_match_seconds = 0.0f;
    int active_peak_lead = 0;
    int active_peak_deficit = 0;
    int official_games_left = 0;
    int official_games_right = 0;
    bool post_forfeit_scene_pending = false;
    bool imagination_takeover_cue_shown = false;
    float imagination_takeover_cue_seconds = 0.0f;
};

}  // namespace whacker::app
