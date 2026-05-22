#include "paddle_tuning_actions.hpp"

#include <algorithm>
#include <cmath>

#include "ai_style_catalog.hpp"
#include "story_skill_limits.hpp"

namespace whacker::app {

namespace {

constexpr int kPaddleTuningComponentCount = 3;

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

whacker::progression::SkillState clamp_skill_components(const whacker::progression::SkillState& skills) {
    whacker::progression::SkillState clamped = skills;
    clamped.edge = clampf(clamped.edge, 0.0f, 1.0f);
    clamped.power = clampf(clamped.power, 0.0f, 1.0f);
    clamped.spin_inject = clampf(clamped.spin_inject, 0.0f, 1.0f);
    return clamped;
}

float component_max(const whacker::progression::SkillState& max_skills, const int component_index) {
    switch (component_index) {
        case 0:
            return max_skills.edge;
        case 1:
            return max_skills.power;
        case 2:
            return max_skills.spin_inject;
        default:
            return max_skills.edge;
    }
}

float* component_ptr(PaddleTuning& tuning, const int component_index) {
    switch (component_index) {
        case 0:
            return &tuning.edge;
        case 1:
            return &tuning.power;
        case 2:
            return &tuning.spin_inject;
        default:
            return &tuning.edge;
    }
}

float tuning_limit_budget(const whacker::progression::SkillState& max_skills, const float requested_budget_limit) {
    const float caps_sum = story_skill_sum(max_skills);
    return clampf(std::min(requested_budget_limit, caps_sum), 0.0f, kPaddleTuningBudgetCap);
}

void clamp_tuning_to_limits(
    PaddleTuning& tuning,
    const whacker::progression::SkillState& max_skills,
    const float max_budget) {
    normalize_paddle_tuning(tuning);
    tuning.edge = std::min(tuning.edge, max_skills.edge);
    tuning.power = std::min(tuning.power, max_skills.power);
    tuning.spin_inject = std::min(tuning.spin_inject, max_skills.spin_inject);

    const float budget_limit = tuning_limit_budget(max_skills, max_budget);
    const float sum = tuning.edge + tuning.power + tuning.spin_inject;
    if (sum > budget_limit && sum > 1.0e-6f) {
        const float scale = budget_limit / sum;
        tuning.edge *= scale;
        tuning.power *= scale;
        tuning.spin_inject *= scale;
    }
    tuning.budget = clampf(tuning.edge + tuning.power + tuning.spin_inject, 0.0f, budget_limit);
}

bool adjust_component(
    PaddleTuningState& tuning_state,
    const int direction) {
    if (direction == 0) {
        return false;
    }
    float* selected_value = component_ptr(tuning_state.working, tuning_state.selected_component);
    if (selected_value == nullptr) {
        return false;
    }
    const float before = *selected_value;
    if (direction < 0) {
        *selected_value = std::max(0.0f, *selected_value - kPaddleTuningBarStep);
    } else {
        const float other_sum = std::max(
            0.0f,
            (tuning_state.working.edge + tuning_state.working.power + tuning_state.working.spin_inject) -
                *selected_value);
        const float budget_limit = tuning_limit_budget(tuning_state.max_skills, tuning_state.max_budget);
        const float max_for_selected =
            std::min(component_max(tuning_state.max_skills, tuning_state.selected_component),
                std::max(0.0f, budget_limit - other_sum));
        *selected_value = std::min(max_for_selected, *selected_value + kPaddleTuningBarStep);
    }
    clamp_tuning_to_limits(tuning_state.working, tuning_state.max_skills, tuning_state.max_budget);
    return std::abs(*selected_value - before) > 1.0e-6f;
}

int held_horizontal_direction(const MenuIntent& held_intent) {
    if (held_intent.left == held_intent.right) {
        return 0;
    }
    return held_intent.left ? -1 : 1;
}

bool should_fire_horizontal_hold_repeat(PaddleTuningState& tuning_state, const int hold_direction) {
    if (hold_direction == 0) {
        tuning_state.horizontal_hold_direction = 0;
        tuning_state.horizontal_hold_frames = 0;
        return false;
    }
    if (tuning_state.horizontal_hold_direction != hold_direction) {
        tuning_state.horizontal_hold_direction = hold_direction;
        tuning_state.horizontal_hold_frames = 0;
        return false;
    }
    ++tuning_state.horizontal_hold_frames;
    if (tuning_state.horizontal_hold_frames < kPaddleTuningHoldRepeatDelayFrames) {
        return false;
    }
    const int repeat_frames = tuning_state.horizontal_hold_frames - kPaddleTuningHoldRepeatDelayFrames;
    return (repeat_frames % kPaddleTuningHoldRepeatIntervalFrames) == 0;
}

void initialize_tuning_state(
    PaddleTuningState& tuning_state,
    const PaddleTuningTarget target,
    const whacker::progression::SkillState& skills,
    const whacker::progression::SkillState& max_skills,
    const float max_budget) {
    const whacker::progression::SkillState clamped_max = clamp_skill_components(max_skills);
    tuning_state.active = true;
    tuning_state.target = target;
    tuning_state.selected_component = 0;
    tuning_state.horizontal_hold_direction = 0;
    tuning_state.horizontal_hold_frames = 0;
    tuning_state.max_skills = clamped_max;
    tuning_state.max_budget = tuning_limit_budget(clamped_max, max_budget);
    tuning_state.working = paddle_tuning_from_skills(skills);
    clamp_tuning_to_limits(tuning_state.working, tuning_state.max_skills, tuning_state.max_budget);
}

}  // namespace

void begin_quick_paddle_tuning(
    PaddleTuningState& tuning_state,
    const PaddleTuningTarget target,
    const whacker::progression::SkillState& skills) {
    static const whacker::progression::SkillState kQuickTuningMaxSkills {
        .edge = 1.0f,
        .power = 1.0f,
        .spin_inject = 1.0f,
    };
    initialize_tuning_state(
        tuning_state,
        target,
        skills,
        kQuickTuningMaxSkills,
        kPaddleTuningBudgetCap);
}

void begin_story_player_paddle_tuning(
    PaddleTuningState& tuning_state,
    const StoryCareerData& career) {
    initialize_tuning_state(
        tuning_state,
        PaddleTuningTarget::StoryPlayer,
        career.player_skills,
        career.player_skill_caps,
        story_skill_sum(career.player_skill_caps));
}

PaddleTuningActionResult apply_paddle_tuning_action(
    PaddleTuningState& tuning_state,
    const PaddleTuningInputIntent& intent) {
    bool changed = false;
    if (intent.pressed.up) {
        tuning_state.selected_component =
            (tuning_state.selected_component + kPaddleTuningComponentCount - 1) % kPaddleTuningComponentCount;
        changed = true;
    }
    if (intent.pressed.down) {
        tuning_state.selected_component =
            (tuning_state.selected_component + 1) % kPaddleTuningComponentCount;
        changed = true;
    }

    int direction = 0;
    if (intent.pressed.left) {
        direction -= 1;
    }
    if (intent.pressed.right) {
        direction += 1;
    }
    if (direction != 0) {
        changed = adjust_component(tuning_state, direction) || changed;
    } else if (should_fire_horizontal_hold_repeat(tuning_state, held_horizontal_direction(intent.held))) {
        changed = adjust_component(tuning_state, tuning_state.horizontal_hold_direction) || changed;
    }

    if (intent.pressed.confirm) {
        return PaddleTuningActionResult::Commit;
    }
    if (intent.pressed.back || intent.pause) {
        return PaddleTuningActionResult::Cancel;
    }
    return changed ? PaddleTuningActionResult::Changed : PaddleTuningActionResult::None;
}

void commit_paddle_tuning_to_options(
    const PaddleTuningState& tuning_state,
    MatchOptions& options) {
    whacker::progression::SkillState skills = paddle_tuning_to_skills(tuning_state.working);
    whacker::progression::clamp_skills(skills);
    if (tuning_state.target == PaddleTuningTarget::QuickLeft) {
        options.left_paddle_skills = skills;
        options.left_ai_style = style_for_skills(skills);
    } else if (tuning_state.target == PaddleTuningTarget::QuickRight) {
        options.right_paddle_skills = skills;
        options.right_ai_style = style_for_skills(skills);
    }
}

void commit_paddle_tuning_to_career(
    const PaddleTuningState& tuning_state,
    StoryCareerData& career) {
    whacker::progression::SkillState skills = paddle_tuning_to_skills(tuning_state.working);
    whacker::progression::clamp_skills(skills);
    career.player_skills = skills;
    normalize_story_player_skill_progress(career.player_skills, career.player_skill_caps);
}

}  // namespace whacker::app
