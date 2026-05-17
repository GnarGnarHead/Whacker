#include "progression/tags.hpp"

#include <algorithm>

namespace whacker::progression {

namespace {

float clamp01(const float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

template <typename T>
bool contains(const std::vector<T>& tags, const T value) {
    return std::find(tags.begin(), tags.end(), value) != tags.end();
}

}  // namespace

std::vector<TrainingTag> emit_training_tags(const TrainingBlockSummary& in_summary) {
    TrainingBlockSummary summary = in_summary;
    summary.training_match_count = std::max(0, summary.training_match_count);
    summary.training_minutes_total = std::max(0.0f, summary.training_minutes_total);
    summary.edge_usage_mean = clamp01(summary.edge_usage_mean);
    summary.power_usage_mean = clamp01(summary.power_usage_mean);
    summary.spin_inject_usage_mean = clamp01(summary.spin_inject_usage_mean);
    summary.clean_contact_rate_mean = clamp01(summary.clean_contact_rate_mean);
    summary.high_edge_contact_rate_mean = clamp01(summary.high_edge_contact_rate_mean);

    std::vector<TrainingTag> out;
    out.reserve(2);

    const bool grinder =
        (summary.training_match_count >= 8) || (summary.training_minutes_total >= 25.0f);
    const bool slacker =
        (summary.training_match_count <= 2) && (summary.training_minutes_total <= 8.0f);
    if (grinder || slacker) {
        if (grinder && slacker) {
            out.push_back(summary.training_match_count >= 5 ? TrainingTag::Grinder : TrainingTag::Slacker);
        } else {
            out.push_back(grinder ? TrainingTag::Grinder : TrainingTag::Slacker);
        }
    }

    const bool technician =
        (summary.edge_usage_mean >= 0.62f) &&
        (summary.clean_contact_rate_mean >= 0.52f);
    const bool power_specialist =
        (summary.power_usage_mean >= 0.62f) &&
        (summary.spin_inject_usage_mean <= 0.58f);
    const bool spin_specialist =
        (summary.spin_inject_usage_mean >= 0.64f) &&
        (summary.power_usage_mean <= 0.60f);

    float best_style_score = -1.0e9f;
    TrainingTag best_style = TrainingTag::Technician;
    bool has_style = false;

    if (technician) {
        const float score = std::min(
            summary.edge_usage_mean - 0.62f,
            summary.clean_contact_rate_mean - 0.52f);
        if (!has_style || (score > best_style_score)) {
            has_style = true;
            best_style = TrainingTag::Technician;
            best_style_score = score;
        }
    }
    if (power_specialist) {
        const float score = std::min(summary.power_usage_mean - 0.62f, 0.58f - summary.spin_inject_usage_mean);
        if (!has_style || (score > best_style_score)) {
            has_style = true;
            best_style = TrainingTag::PowerSpecialist;
            best_style_score = score;
        }
    }
    if (spin_specialist) {
        const float score = std::min(summary.spin_inject_usage_mean - 0.64f, 0.60f - summary.power_usage_mean);
        if (!has_style || (score > best_style_score)) {
            has_style = true;
            best_style = TrainingTag::SpinSpecialist;
        }
    }

    if (has_style && out.size() < 2) {
        out.push_back(best_style);
    }

    const bool reckless =
        (summary.high_edge_contact_rate_mean >= 0.48f) &&
        (summary.clean_contact_rate_mean <= 0.42f);
    if (reckless && out.size() < 2 && !contains(out, TrainingTag::Reckless)) {
        out.push_back(TrainingTag::Reckless);
    }

    return out;
}

std::vector<OfficialTag> emit_official_tags(const OfficialMatchSummary& summary) {
    std::vector<OfficialTag> out;
    out.reserve(3);

    const int margin = std::abs(summary.score_for - summary.score_against);
    const bool down_two_then_win = (summary.peak_deficit >= 2) && summary.won;
    const bool up_two_then_loss = (summary.peak_lead >= 2) && !summary.won;

    if (down_two_then_win) {
        out.push_back(OfficialTag::ComebackWin);
    } else if (up_two_then_loss) {
        out.push_back(OfficialTag::ChokeLoss);
    }

    if (summary.won && (summary.expected_win_prob <= 0.35f)) {
        if (out.size() < 3) {
            out.push_back(OfficialTag::UpsetWin);
        }
    } else if (!summary.won && (summary.expected_win_prob <= 0.45f)) {
        if (out.size() < 3) {
            out.push_back(OfficialTag::ExpectedLoss);
        }
    }

    if (summary.won && (margin >= 3) && (summary.score_against <= 2)) {
        if (out.size() < 3) {
            out.push_back(OfficialTag::DominantVictory);
        }
    } else if (!summary.won && (margin == 1)) {
        if (out.size() < 3) {
            out.push_back(OfficialTag::NarrowDefeat);
        }
    }

    return out;
}

const char* to_string(const TrainingTag tag) {
    switch (tag) {
        case TrainingTag::Grinder:
            return "GRINDER";
        case TrainingTag::Slacker:
            return "SLACKER";
        case TrainingTag::Technician:
            return "TECHNICIAN";
        case TrainingTag::PowerSpecialist:
            return "POWER_SPECIALIST";
        case TrainingTag::SpinSpecialist:
            return "SPIN_SPECIALIST";
        case TrainingTag::Reckless:
            return "RECKLESS";
    }
    return "UNKNOWN_TRAINING_TAG";
}

const char* to_string(const OfficialTag tag) {
    switch (tag) {
        case OfficialTag::UpsetWin:
            return "UPSET_WIN";
        case OfficialTag::ExpectedLoss:
            return "EXPECTED_LOSS";
        case OfficialTag::ComebackWin:
            return "COMEBACK_WIN";
        case OfficialTag::ChokeLoss:
            return "CHOKE_LOSS";
        case OfficialTag::DominantVictory:
            return "DOMINANT_VICTORY";
        case OfficialTag::NarrowDefeat:
            return "NARROW_DEFEAT";
    }
    return "UNKNOWN_OFFICIAL_TAG";
}

}  // namespace whacker::progression
