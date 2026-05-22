#include "sdl_runtime_audio.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "sim/math.hpp"

namespace whacker::app {

namespace {

bool crossed_zero(const float before, const float after) {
    return (before > 0.0f && after < 0.0f) || (before < 0.0f && after > 0.0f);
}

bool detect_wall_bounce(
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const whacker::sim::SimulationConfig& config) {
    if (!crossed_zero(before.ball.velocity.y, after.ball.velocity.y)) {
        return false;
    }
    const float top_y = config.ball_radius + 0.6f;
    const float bottom_y = config.court_height - config.ball_radius - 0.6f;
    return after.ball.position.y <= top_y || after.ball.position.y >= bottom_y;
}

PaddleHitAudioParams build_paddle_hit_audio_params(
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const whacker::sim::SimulationConfig& config) {
    const bool hitter_left = after.ball.velocity.x > 0.0f;
    const whacker::sim::PaddleState& paddle = hitter_left ? after.left : after.right;

    const float denom = std::max(config.paddle_half_height, 1.0e-3f);
    const float contact_u = whacker::sim::clampf((after.ball.position.y - paddle.center_y) / denom, -1.0f, 1.0f);
    const float center = whacker::sim::clamp01(1.0f - std::abs(contact_u));
    const float paddle_max = whacker::sim::paddle_max_speed_for(config, paddle);
    const float impact = whacker::sim::impact_power_factor(paddle.velocity_y, paddle_max);
    const float power_skill = whacker::sim::power_scale_for(paddle);
    const float technical_skill = whacker::sim::technical_scale_for(paddle);
    const float spin_skill = whacker::sim::spin_scale_for(paddle);

    const float speed_scalar_gain = whacker::sim::clamp01((after.ball.speed_scalar - before.ball.speed_scalar) / 0.30f);
    const float power = whacker::sim::clamp01(
        (0.55f * center * (0.35f + (0.65f * impact)) * (0.25f + (0.75f * power_skill))) +
        (0.45f * speed_scalar_gain));

    const float technical_contact =
        whacker::sim::technical_scale_at_contact(config, technical_skill, power_skill, impact);
    const float angle = whacker::sim::clamp01(std::abs(contact_u) * (0.28f + (0.72f * technical_contact)));

    const float spin_delta = after.ball.spin - before.ball.spin;
    const float spin_abs_norm =
        whacker::sim::clamp01(std::abs(after.ball.spin) / std::max(config.spin_max, 1.0e-3f));
    const float spin_delta_norm =
        whacker::sim::clamp01(std::abs(spin_delta) / std::max(config.spin_max, 1.0e-3f));
    const float spin_contact =
        whacker::sim::spin_scale_at_contact(config, spin_skill, power_skill, impact);
    const float spin_motion =
        whacker::sim::clamp01(std::abs(paddle.velocity_y) / std::max(paddle_max, 1.0e-3f));
    const float spin = whacker::sim::clamp01(
        (0.55f * spin_abs_norm) +
        (0.30f * spin_delta_norm) +
        (0.15f * whacker::sim::clamp01(spin_contact * spin_motion)));

    float spin_sign = 0.0f;
    if (std::abs(after.ball.spin) > 1.0e-4f) {
        spin_sign = after.ball.spin > 0.0f ? 1.0f : -1.0f;
    } else if (std::abs(spin_delta) > 1.0e-4f) {
        spin_sign = spin_delta > 0.0f ? 1.0f : -1.0f;
    } else if (std::abs(paddle.velocity_y) > 1.0e-4f) {
        spin_sign = paddle.velocity_y > 0.0f ? 1.0f : -1.0f;
    } else {
        spin_sign = after.ball.spin >= 0.0f ? 1.0f : -1.0f;
    }

    return PaddleHitAudioParams {.power = power, .angle = angle, .spin = spin, .spin_sign = spin_sign};
}

WallHitAudioParams build_wall_hit_audio_params(
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const whacker::sim::SimulationConfig& config) {
    const float pre_speed = whacker::sim::speed_of(before.ball);
    const float speed_ref = std::max(config.ball_base_speed, 1.0e-3f);
    const float normal_ratio =
        whacker::sim::clamp01(std::abs(before.ball.velocity.y) / std::max(pre_speed, 1.0e-3f));
    const float speed_ratio = whacker::sim::clamp01((pre_speed / speed_ref - 0.45f) / 1.35f);
    const float impact = whacker::sim::clamp01((0.45f * normal_ratio) + (0.55f * speed_ratio));

    const float spin_before = whacker::sim::clamp01(std::abs(before.ball.spin) / std::max(config.spin_max, 1.0e-3f));
    const float spin_transfer =
        whacker::sim::clamp01(std::abs(after.ball.spin - before.ball.spin) / std::max(config.spin_max, 1.0e-3f));
    const float spin = whacker::sim::clamp01((0.62f * spin_before) + (0.38f * spin_transfer));

    float spin_sign = 0.0f;
    if (std::abs(before.ball.spin) > 1.0e-4f) {
        spin_sign = before.ball.spin > 0.0f ? 1.0f : -1.0f;
    } else {
        spin_sign = before.ball.velocity.x >= 0.0f ? 1.0f : -1.0f;
    }

    return WallHitAudioParams {.impact = impact, .spin = spin, .spin_sign = spin_sign};
}

float next_type_blip_cooldown(std::uint32_t& type_blip_pattern_step) {
    static constexpr std::array<float, 8> kRhythmSeconds {
        0.040f, 0.055f, 0.043f, 0.061f, 0.046f, 0.052f, 0.041f, 0.058f};
    const std::uint32_t step = type_blip_pattern_step++;
    float cooldown = kRhythmSeconds[step % static_cast<std::uint32_t>(kRhythmSeconds.size())];
    if (step % 9u == 4u) {
        cooldown += 0.010f;
    }
    return cooldown;
}

}  // namespace

void initialize_sdl_runtime_audio(SdlRuntimeState& runtime) {
    runtime.audio_settings = clamp_audio_settings(runtime.audio_settings);
    (void)runtime.audio_engine.init();
    runtime.audio_engine.set_settings(runtime.audio_settings);
}

void shutdown_sdl_runtime_audio(SdlRuntimeState& runtime) {
    runtime.audio_engine.shutdown();
}

void apply_sdl_runtime_audio_settings(SdlRuntimeState& runtime) {
    runtime.audio_settings = clamp_audio_settings(runtime.audio_settings);
    runtime.audio_engine.set_settings(runtime.audio_settings);
}

void play_menu_move_sound(SdlRuntimeState& runtime) {
    runtime.audio_engine.push_event(AudioEventId::MenuMove);
}

void play_menu_confirm_sound(SdlRuntimeState& runtime) {
    runtime.audio_engine.push_event(AudioEventId::MenuConfirm);
}

void tick_story_typewriter_audio(SdlRuntimeState& runtime, const float dt_seconds) {
    runtime.type_blip_cooldown = std::max(0.0f, runtime.type_blip_cooldown - dt_seconds);
}

void route_story_typewriter_audio(
    SdlRuntimeState& runtime,
    const bool dialogue_writing,
    const std::size_t visible_before,
    const std::size_t visible_after) {
    if (!dialogue_writing || visible_after <= visible_before || runtime.type_blip_cooldown > 0.0f) {
        return;
    }
    runtime.audio_engine.push_event(AudioEventId::TypeBlip);
    runtime.type_blip_cooldown = next_type_blip_cooldown(runtime.type_blip_pattern_step);
}

void play_match_opening_countdown_cue(SdlRuntimeState& runtime) {
    runtime.audio_engine.push_event(AudioEventId::ServeBlink);
}

void route_step_audio_events(
    SdlRuntimeState& runtime,
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const whacker::sim::ScoreEvent score_event,
    const whacker::sim::SimulationConfig& config) {
    if (after.rally_hits > before.rally_hits) {
        runtime.audio_engine.push_paddle_hit(build_paddle_hit_audio_params(before, after, config));
    } else if (score_event == whacker::sim::ScoreEvent::None && detect_wall_bounce(before, after, config)) {
        runtime.audio_engine.push_wall_hit(build_wall_hit_audio_params(before, after, config));
    }
    if (score_event != whacker::sim::ScoreEvent::None) {
        runtime.audio_engine.push_event(AudioEventId::Score);
    }
}

}  // namespace whacker::app
