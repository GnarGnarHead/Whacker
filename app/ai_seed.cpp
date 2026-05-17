#include "ai_seed.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "ai_frame.hpp"

namespace whacker::app::ai_internal {

namespace {

constexpr float kInboundEpsilon = 1.0e-4f;

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

float clamp01f(const float value) {
    return clampf(value, 0.0f, 1.0f);
}

int clampi(const int value, const int lo, const int hi) {
    return std::max(lo, std::min(value, hi));
}

uint32_t mix32(std::uint64_t value) {
    value ^= (value >> 33U);
    value *= 0xff51afd7ed558ccdULL;
    value ^= (value >> 33U);
    value *= 0xc4ceb9fe1a85ec53ULL;
    value ^= (value >> 33U);
    return static_cast<uint32_t>(value & 0xFFFFFFFFULL);
}

uint32_t hash_combine32(const uint32_t seed, const uint32_t value) {
    constexpr uint32_t kMul = 0x9e3779b9U;
    const uint32_t mixed = value + kMul + (seed << 6U) + (seed >> 2U);
    return seed ^ mixed;
}

uint64_t hash_combine64(const uint64_t seed, const uint64_t value) {
    return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6U) + (seed >> 2U));
}

uint32_t quantize_to_u32(const float value, const float scale) {
    const float scaled = value * scale;
    const int rounded = static_cast<int>(std::lround(scaled));
    return static_cast<uint32_t>(rounded);
}

uint32_t style_to_u32(const AiStyle style) {
    return static_cast<uint32_t>(static_cast<std::uint8_t>(style));
}

uint64_t compute_state_signature_actor_context(
    const whacker::sim::RallyState& state,
    const whacker::sim::SimulationConfig& config) {
    uint64_t seed = 0xBB67AE8584CAA73BULL;
    seed = hash_combine64(seed, static_cast<uint64_t>(state.left_score));
    seed = hash_combine64(seed, static_cast<uint64_t>(state.right_score));
    seed = hash_combine64(seed, state.rally_hits);

    const bool inbound = state.ball.velocity.x < -kInboundEpsilon;
    seed = hash_combine64(seed, inbound ? 1ULL : 0ULL);

    const float contact_plane_x = config.paddle_x_margin + config.paddle_half_width + config.ball_radius;
    const float inbound_distance = std::max(0.0f, state.ball.position.x - contact_plane_x);
    const float distance_norm = inbound_distance / std::max(config.court_width, 1.0e-3f);
    const int approach_bucket = inbound
        ? clampi(static_cast<int>(std::floor(distance_norm * 6.0f)), 0, 5)
        : 0;
    seed = hash_combine64(seed, static_cast<uint64_t>(approach_bucket));
    return seed;
}

}  // namespace

uint32_t make_noise_base(
    const whacker::sim::RallyState& state,
    const AiStyle style,
    const whacker::progression::SkillState& skills) {
    const uint32_t my_score = static_cast<uint32_t>(state.left_score);
    const uint32_t opponent_score = static_cast<uint32_t>(state.right_score);
    uint32_t seed = 0xA341316CU;
    seed = hash_combine32(seed, my_score);
    seed = hash_combine32(seed, opponent_score);
    seed = hash_combine32(seed, mix32(state.rally_hits));
    seed = hash_combine32(seed, style_to_u32(style));
    seed = hash_combine32(seed, quantize_to_u32(clamp01f(skills.edge), 1000.0f));
    seed = hash_combine32(seed, quantize_to_u32(clamp01f(skills.power), 1000.0f));
    seed = hash_combine32(seed, quantize_to_u32(clamp01f(skills.spin_inject), 1000.0f));
    return seed;
}

float keyed_noise_u01(
    const uint32_t base_seed,
    const std::uint64_t decision_counter,
    const int phase_id,
    const int candidate_id,
    const int draw_id) {
    uint32_t key = base_seed;
    key = hash_combine32(key, mix32(decision_counter));
    key = hash_combine32(key, static_cast<uint32_t>(phase_id));
    key = hash_combine32(key, static_cast<uint32_t>(candidate_id));
    key = hash_combine32(key, static_cast<uint32_t>(draw_id));
    const uint32_t masked = key & 0x00FFFFFFU;
    return static_cast<float>(masked) / 16777215.0f;
}

uint64_t compute_state_signature_from_rally_state(
    const whacker::sim::RallyState& state,
    const bool for_left_paddle) {
    // Legacy compatibility path used by existing symmetry tests and older call
    // sites: keep this intentionally coarse and actor-side normalized.
    uint64_t seed = 0x9E3779B97F4A7C15ULL;
    const uint32_t my_score =
        static_cast<uint32_t>(for_left_paddle ? state.left_score : state.right_score);
    const uint32_t opponent_score =
        static_cast<uint32_t>(for_left_paddle ? state.right_score : state.left_score);
    seed = hash_combine64(seed, static_cast<uint64_t>(my_score));
    seed = hash_combine64(seed, static_cast<uint64_t>(opponent_score));
    seed = hash_combine64(seed, state.rally_hits);
    return seed;
}

uint64_t compute_state_signature_from_simulation(
    const whacker::sim::Simulation& simulation,
    const bool for_left_paddle) {
    const whacker::sim::Simulation actor_simulation = make_actor_frame_simulation(simulation, for_left_paddle);
    return compute_state_signature_actor_context(actor_simulation.state(), actor_simulation.config());
}

}  // namespace whacker::app::ai_internal
