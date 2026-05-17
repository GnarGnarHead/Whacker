#pragma once

#include <algorithm>
#include <cmath>

#include "ai_core.hpp"

namespace whacker::app::ai_internal {

constexpr float kInboundEpsilon = 1.0e-4f;
constexpr float kMinReachEpsilon = 0.75f;
constexpr float kScoreEpsilon = 1.0e-6f;
constexpr int kPhaseJitter = 1;
constexpr int kPhaseCooldown = 2;
constexpr int kPhaseTieBreak = 3;

inline float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

inline float clamp01f(const float value) {
    return clampf(value, 0.0f, 1.0f);
}

inline int clampi(const int value, const int lo, const int hi) {
    return std::max(lo, std::min(value, hi));
}

inline float signf(const float value) {
    if (value > 0.0f) {
        return 1.0f;
    }
    if (value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

inline float lerpf(const float a, const float b, const float t) {
    return a + ((b - a) * t);
}

struct ReachabilityEnvelope {
    float min_center_y = 0.0f;
    float max_center_y = 0.0f;
};

struct PredictorResult {
    bool predicted = false;
    float t_hit = 0.0f;
    float intercept_y = 0.0f;
    float intercept_vy = 0.0f;
    int wall_bounces = 0;
    float confidence = 0.0f;
};

struct StyleMix {
    float power = 0.0f;
    float technical = 0.0f;
    float spin = 0.0f;
};

struct IntentWeights {
    float stabilize = 0.0f;
    float pressure = 0.0f;
    float spintrap = 0.0f;
};

struct AiCapabilityProfile {
    float reaction_lag_s = 0.0f;
    float target_quantization_step = 0.5f;
    float intercept_jitter_amplitude = 0.0f;
    float speed_scale = 1.0f;
    float accel_scale = 1.0f;
    float makeability_scale = 1.0f;
};

struct Candidate {
    int id = -1;
    AiIntent intent = AiIntent::Stabilize;
    float contact_u = 0.0f;
    float strike_vy = 0.0f;
    float required_center_y = 0.0f;
    float required_speed_ratio = 0.0f;
    float reach_slack = 0.0f;
    float make_term = 0.0f;
    float quality_term = 0.0f;
    float style_term = 0.0f;
    float risk_term = 0.0f;
    float motion_term = 0.0f;
    float score = 0.0f;
    float impact_factor = 0.0f;
    float spin_delta_estimate = 0.0f;
    float clean_contact_metric = 0.0f;
    float cheap_score = 0.0f;
};

}  // namespace whacker::app::ai_internal
