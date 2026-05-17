#pragma once

#include <algorithm>

#include "progression/skills.hpp"

namespace whacker::app {

inline constexpr whacker::progression::SkillState kStoryPlayerStarterSkills {
    .edge = 0.10f,
    .power = 0.10f,
    .spin_inject = 0.10f,
};

inline float story_skill_sum(const whacker::progression::SkillState& skills) {
    return skills.edge + skills.power + skills.spin_inject;
}

inline void clamp_story_player_skill_caps(whacker::progression::SkillState& caps) {
    whacker::progression::clamp_skills(caps);
}

inline void clamp_story_player_skills_to_caps(
    whacker::progression::SkillState& skills,
    const whacker::progression::SkillState& caps) {
    whacker::progression::clamp_skills(skills);
    skills.edge = std::clamp(skills.edge, 0.0f, caps.edge);
    skills.power = std::clamp(skills.power, 0.0f, caps.power);
    skills.spin_inject = std::clamp(skills.spin_inject, 0.0f, caps.spin_inject);
}

inline void normalize_story_player_skill_progress(
    whacker::progression::SkillState& skills,
    whacker::progression::SkillState& caps) {
    clamp_story_player_skill_caps(caps);
    clamp_story_player_skills_to_caps(skills, caps);
}

}  // namespace whacker::app
