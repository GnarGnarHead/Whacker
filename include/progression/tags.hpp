#pragma once

#include <cstdint>
#include <vector>

namespace whacker::progression {

enum class TrainingTag : std::uint8_t {
    Grinder,
    Slacker,
    Technician,
    PowerSpecialist,
    SpinSpecialist,
    Reckless
};

enum class OfficialTag : std::uint8_t {
    UpsetWin,
    ExpectedLoss,
    ComebackWin,
    ChokeLoss,
    DominantVictory,
    NarrowDefeat
};

struct TrainingBlockSummary {
    int training_match_count = 0;
    float training_minutes_total = 0.0f;
    float edge_usage_mean = 0.0f;
    float power_usage_mean = 0.0f;
    float spin_inject_usage_mean = 0.0f;
    float clean_contact_rate_mean = 0.0f;
    float high_edge_contact_rate_mean = 0.0f;
};

struct OfficialMatchSummary {
    bool won = false;
    int score_for = 0;
    int score_against = 0;
    int peak_deficit = 0;
    int peak_lead = 0;
    float expected_win_prob = 0.5f;
};

std::vector<TrainingTag> emit_training_tags(const TrainingBlockSummary& summary);
std::vector<OfficialTag> emit_official_tags(const OfficialMatchSummary& summary);

const char* to_string(TrainingTag tag);
const char* to_string(OfficialTag tag);

}  // namespace whacker::progression
