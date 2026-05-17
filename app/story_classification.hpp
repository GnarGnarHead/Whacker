#pragma once

#include "progression/skills.hpp"
#include "story_state.hpp"

namespace whacker::app {

StoryIntroStyleHint classify_story_style_hint(const whacker::progression::SkillUsageAccumulator& usage_acc);
StoryIntroStyleHint classify_story_weakness_hint(const whacker::progression::SkillUsageMetrics& usage);
StoryIntroPerformanceHint classify_story_performance_hint(bool player_won, int player_score, int opponent_score);

}  // namespace whacker::app

