#pragma once

#include <cstdint>

#include "app_types.hpp"
#include "ui_state.hpp"

namespace whacker::app {

constexpr float kPaddleTuningBudgetCap = whacker::progression::kSkillBudgetCap;
constexpr float kPaddleTuningBalancedThreshold = 0.08f;
constexpr float kPaddleTuningNudgeStep = 0.08f;
constexpr float kPaddleTuningBarStep = 0.01f;
constexpr int kPaddleTuningHoldRepeatDelayFrames = 12;
constexpr int kPaddleTuningHoldRepeatIntervalFrames = 2;

struct PaddleTuning {
    float edge = 1.0f / 3.0f;
    float power = 1.0f / 3.0f;
    float spin_inject = 1.0f / 3.0f;
    float budget = 0.84f;
};

struct PaddleTuningPoint2D {
    float x = 0.0f;
    float y = 0.0f;
};

enum class PaddleTuningTarget : std::uint8_t {
    QuickLeft = 0,
    QuickRight = 1,
    StoryPlayer = 2
};

struct PaddleTuningState {
    bool active = false;
    AppState return_state = AppState::QuickMatchSetup;
    PaddleTuningTarget target = PaddleTuningTarget::QuickLeft;
    int selected_component = 0;
    int horizontal_hold_direction = 0;
    int horizontal_hold_frames = 0;
    whacker::progression::SkillState max_skills {.edge = 1.0f, .power = 1.0f, .spin_inject = 1.0f};
    float max_budget = kPaddleTuningBudgetCap;
    PaddleTuning working {};
};

void normalize_paddle_tuning(PaddleTuning& tuning);
PaddleTuning paddle_tuning_from_skills(const whacker::progression::SkillState& skills);
whacker::progression::SkillState paddle_tuning_to_skills(const PaddleTuning& tuning);
AiStyle paddle_tuning_style(const PaddleTuning& tuning);
AiStyle style_for_skills(const whacker::progression::SkillState& skills);
void nudge_paddle_tuning(PaddleTuning& tuning, float delta_x, float delta_y);
PaddleTuningPoint2D paddle_tuning_point(const PaddleTuning& tuning);
const char* paddle_tuning_target_title(PaddleTuningTarget target);

}  // namespace whacker::app
