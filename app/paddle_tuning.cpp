#include "paddle_tuning.hpp"

#include <algorithm>
#include <cmath>

namespace whacker::app {

namespace {

struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;
};

constexpr float kSqrt3Over2 = 0.8660254037844386f;
constexpr Vec2f kVertexEdge {0.0f, 1.0f};
constexpr Vec2f kVertexPower {-kSqrt3Over2, -0.5f};
constexpr Vec2f kVertexSpin {kSqrt3Over2, -0.5f};

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

Vec2f point_for_weights(const float edge, const float power, const float spin_inject) {
    return Vec2f {
        (edge * kVertexEdge.x) + (power * kVertexPower.x) + (spin_inject * kVertexSpin.x),
        (edge * kVertexEdge.y) + (power * kVertexPower.y) + (spin_inject * kVertexSpin.y)
    };
}

void barycentric_weights_for_point(
    const Vec2f point,
    float& edge_out,
    float& power_out,
    float& spin_out) {
    const float x1 = kVertexEdge.x;
    const float y1 = kVertexEdge.y;
    const float x2 = kVertexPower.x;
    const float y2 = kVertexPower.y;
    const float x3 = kVertexSpin.x;
    const float y3 = kVertexSpin.y;
    const float denom = ((y2 - y3) * (x1 - x3)) + ((x3 - x2) * (y1 - y3));
    if (std::abs(denom) <= 1.0e-6f) {
        edge_out = 1.0f / 3.0f;
        power_out = 1.0f / 3.0f;
        spin_out = 1.0f / 3.0f;
        return;
    }
    const float inv_denom = 1.0f / denom;
    edge_out = (((y2 - y3) * (point.x - x3)) + ((x3 - x2) * (point.y - y3))) * inv_denom;
    power_out = (((y3 - y1) * (point.x - x3)) + ((x1 - x3) * (point.y - y3))) * inv_denom;
    spin_out = 1.0f - edge_out - power_out;
}

void normalize_weights(float& edge, float& power, float& spin_inject) {
    edge = std::max(0.0f, edge);
    power = std::max(0.0f, power);
    spin_inject = std::max(0.0f, spin_inject);
    const float sum = edge + power + spin_inject;
    if (sum <= 1.0e-6f) {
        edge = 1.0f / 3.0f;
        power = 1.0f / 3.0f;
        spin_inject = 1.0f / 3.0f;
        return;
    }
    const float inv_sum = 1.0f / sum;
    edge *= inv_sum;
    power *= inv_sum;
    spin_inject *= inv_sum;
}

float sum_components(const PaddleTuning& tuning) {
    return tuning.edge + tuning.power + tuning.spin_inject;
}

void normalized_weights_from_tuning(
    const PaddleTuning& tuning,
    float& edge_weight,
    float& power_weight,
    float& spin_weight) {
    edge_weight = std::max(0.0f, tuning.edge);
    power_weight = std::max(0.0f, tuning.power);
    spin_weight = std::max(0.0f, tuning.spin_inject);
    normalize_weights(edge_weight, power_weight, spin_weight);
}

}  // namespace

void normalize_paddle_tuning(PaddleTuning& tuning) {
    tuning.edge = clampf(tuning.edge, 0.0f, 1.0f);
    tuning.power = clampf(tuning.power, 0.0f, 1.0f);
    tuning.spin_inject = clampf(tuning.spin_inject, 0.0f, 1.0f);

    const float sum = sum_components(tuning);
    if (sum > kPaddleTuningBudgetCap && sum > 1.0e-6f) {
        const float scale = kPaddleTuningBudgetCap / sum;
        tuning.edge *= scale;
        tuning.power *= scale;
        tuning.spin_inject *= scale;
    }

    tuning.budget = clampf(sum_components(tuning), 0.0f, kPaddleTuningBudgetCap);
}

PaddleTuning paddle_tuning_from_skills(const whacker::progression::SkillState& skills) {
    PaddleTuning tuning {};
    tuning.edge = skills.edge;
    tuning.power = skills.power;
    tuning.spin_inject = skills.spin_inject;
    normalize_paddle_tuning(tuning);
    return tuning;
}

whacker::progression::SkillState paddle_tuning_to_skills(const PaddleTuning& raw_tuning) {
    PaddleTuning tuning = raw_tuning;
    normalize_paddle_tuning(tuning);
    whacker::progression::SkillState skills {};
    skills.edge = tuning.edge;
    skills.power = tuning.power;
    skills.spin_inject = tuning.spin_inject;
    return skills;
}

AiStyle paddle_tuning_style(const PaddleTuning& raw_tuning) {
    PaddleTuning tuning = raw_tuning;
    normalize_paddle_tuning(tuning);
    float edge_weight = 0.0f;
    float power_weight = 0.0f;
    float spin_weight = 0.0f;
    normalized_weights_from_tuning(tuning, edge_weight, power_weight, spin_weight);
    const float max_weight = std::max({edge_weight, power_weight, spin_weight});
    const float min_weight = std::min({edge_weight, power_weight, spin_weight});
    if ((max_weight - min_weight) < kPaddleTuningBalancedThreshold) {
        return AiStyle::Balanced;
    }
    if (power_weight >= edge_weight && power_weight >= spin_weight) {
        return AiStyle::Power;
    }
    if (edge_weight >= power_weight && edge_weight >= spin_weight) {
        return AiStyle::Technical;
    }
    return AiStyle::Spin;
}

AiStyle style_for_skills(const whacker::progression::SkillState& skills) {
    return paddle_tuning_style(paddle_tuning_from_skills(skills));
}

void nudge_paddle_tuning(PaddleTuning& tuning, const float delta_x, const float delta_y) {
    normalize_paddle_tuning(tuning);
    const float budget = tuning.budget;
    float edge_weight = 0.0f;
    float power_weight = 0.0f;
    float spin_weight = 0.0f;
    normalized_weights_from_tuning(tuning, edge_weight, power_weight, spin_weight);
    const Vec2f point = point_for_weights(edge_weight, power_weight, spin_weight);
    const Vec2f nudged {point.x + delta_x, point.y + delta_y};
    float edge = 0.0f;
    float power = 0.0f;
    float spin_inject = 0.0f;
    barycentric_weights_for_point(nudged, edge, power, spin_inject);
    normalize_weights(edge, power, spin_inject);
    tuning.edge = edge * budget;
    tuning.power = power * budget;
    tuning.spin_inject = spin_inject * budget;
    normalize_paddle_tuning(tuning);
}

PaddleTuningPoint2D paddle_tuning_point(const PaddleTuning& raw_tuning) {
    PaddleTuning tuning = raw_tuning;
    normalize_paddle_tuning(tuning);
    float edge_weight = 0.0f;
    float power_weight = 0.0f;
    float spin_weight = 0.0f;
    normalized_weights_from_tuning(tuning, edge_weight, power_weight, spin_weight);
    const Vec2f point = point_for_weights(edge_weight, power_weight, spin_weight);
    return PaddleTuningPoint2D {.x = point.x, .y = point.y};
}

const char* paddle_tuning_target_title(const PaddleTuningTarget target) {
    switch (target) {
        case PaddleTuningTarget::QuickLeft:
            return "P1 PADDLE TUNING";
        case PaddleTuningTarget::QuickRight:
            return "P2 PADDLE TUNING";
        case PaddleTuningTarget::StoryPlayer:
            return "PLAYER PADDLE TUNING";
        default:
            return "PADDLE TUNING";
    }
}

}  // namespace whacker::app
