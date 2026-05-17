#include "story_classification.hpp"

#include <algorithm>
#include <cmath>

namespace whacker::app {

StoryIntroStyleHint classify_story_style_hint(const whacker::progression::SkillUsageAccumulator& usage_acc) {
    if (usage_acc.contacts < 3) {
        return StoryIntroStyleHint::Balanced;
    }

    const whacker::progression::SkillUsageMetrics usage = whacker::progression::finalize_usage(usage_acc);
    const float max_usage = std::max({usage.power, usage.edge, usage.spin_inject});
    const float min_usage = std::min({usage.power, usage.edge, usage.spin_inject});
    if ((max_usage - min_usage) < 0.08f) {
        return StoryIntroStyleHint::Balanced;
    }

    if (usage.power >= usage.edge && usage.power >= usage.spin_inject) {
        return StoryIntroStyleHint::Power;
    }
    if (usage.edge >= usage.power && usage.edge >= usage.spin_inject) {
        return StoryIntroStyleHint::Technical;
    }
    return StoryIntroStyleHint::Spin;
}

StoryIntroStyleHint classify_story_weakness_hint(const whacker::progression::SkillUsageMetrics& usage) {
    const float max_usage = std::max({usage.power, usage.edge, usage.spin_inject});
    const float min_usage = std::min({usage.power, usage.edge, usage.spin_inject});
    if ((max_usage - min_usage) < 0.08f) {
        return StoryIntroStyleHint::Balanced;
    }

    if (usage.power <= usage.edge && usage.power <= usage.spin_inject) {
        return StoryIntroStyleHint::Power;
    }
    if (usage.edge <= usage.power && usage.edge <= usage.spin_inject) {
        return StoryIntroStyleHint::Technical;
    }
    return StoryIntroStyleHint::Spin;
}

StoryIntroPerformanceHint classify_story_performance_hint(
    const bool player_won,
    const int player_score,
    const int opponent_score) {
    const int margin = std::abs(player_score - opponent_score);
    if (player_won && margin >= 4) {
        return StoryIntroPerformanceHint::BigWin;
    }
    if (!player_won && margin <= 2) {
        return StoryIntroPerformanceHint::CloseLoss;
    }
    return StoryIntroPerformanceHint::Neutral;
}

}  // namespace whacker::app

