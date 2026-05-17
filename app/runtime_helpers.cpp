#include "runtime_helpers.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>
#include <cmath>

#include "ai_style_catalog.hpp"
#include "progression/skills.hpp"
#include "sim/math.hpp"

namespace whacker::app {

namespace {

bool crossed_zero(const float before, const float after) {
    return (before > 0.0f && after < 0.0f) || (before < 0.0f && after > 0.0f);
}

}  // namespace

void track_intro_contact_usage(
    StoryIntroState& story_intro_state,
    const whacker::sim::SimulationConfig& config,
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after) {
    if (after.rally_hits <= before.rally_hits) {
        return;
    }

    const bool hitter_left = after.ball.velocity.x > 0.0f;
    const bool hitter_is_player = story_intro_state.player_is_right ? !hitter_left : hitter_left;
    if (!hitter_is_player) {
        return;
    }

    const float denom = std::max(config.paddle_half_height, 1.0e-3f);
    const auto& player_paddle = story_intro_state.player_is_right ? after.right : after.left;
    const float contact_u = whacker::sim::clampf((after.ball.position.y - player_paddle.center_y) / denom, -1.0f, 1.0f);
    const float ball_speed = whacker::sim::speed_of(after.ball);
    whacker::progression::accumulate_contact_usage(
        story_intro_state.player_usage,
        contact_u,
        player_paddle.velocity_y,
        ball_speed,
        config);
}

const char* mode_name(const PaddleMode mode) {
    return mode == PaddleMode::Human ? "Human" : "AI";
}

const char* row_name(const int row) {
    switch (row) {
        case MenuRowP1:
            return "P1";
        case MenuRowP2:
            return "P2";
        case MenuRowP1Tuning:
            return "P1 TUNING";
        case MenuRowP2Tuning:
            return "P2 TUNING";
        case MenuRowStart:
            return "START";
        default:
            return "?";
    }
}

const char* main_menu_row_name(const int row) {
    switch (row) {
        case MainMenuRowStory:
            return "STORY MODE";
        case MainMenuRowQuick:
            return "QUICK MATCH";
        case MainMenuRowOptions:
            return "OPTIONS";
        case MainMenuRowQuit:
            return "QUIT";
        default:
            return "?";
    }
}

const char* options_menu_row_name(const int row) {
    switch (row) {
        case OptionsMenuRowP1Up:
            return "P1 UP";
        case OptionsMenuRowP1Down:
            return "P1 DOWN";
        case OptionsMenuRowP2Up:
            return "P2 UP";
        case OptionsMenuRowP2Down:
            return "P2 DOWN";
        case OptionsMenuRowMasterVolume:
            return "MASTER VOLUME";
        case OptionsMenuRowMusicVolume:
            return "MUSIC VOLUME";
        case OptionsMenuRowSfxVolume:
            return "SFX VOLUME";
        case OptionsMenuRowMute:
            return "MUTE";
        case OptionsMenuRowBack:
            return "BACK";
        default:
            return "?";
    }
}

const char* story_menu_row_name(const int row) {
    switch (row) {
        case StoryMenuRowContinue:
            return "CONTINUE";
        case StoryMenuRowNewCareer:
            return "NEW CAREER";
        case StoryMenuRowBack:
            return "BACK";
        default:
            return "?";
    }
}

const char* story_hub_row_name(const int row) {
    switch (row) {
        case StoryHubRowOfficialMatch:
            return "NEXT MATCH";
        case StoryHubRowTrainingMatch:
            return "TRAINING MATCH";
        case StoryHubRowNextWeek:
            return "ADVANCE STORY";
        case StoryHubRowPaddleTuning:
            return "PADDLE TUNING";
        case StoryHubRowBack:
            return "BACK";
        default:
            return "?";
    }
}

const char* story_match_kind_name(const StoryMatchKind kind) {
    switch (kind) {
        case StoryMatchKind::Training:
            return "training";
        case StoryMatchKind::Official:
            return "official";
        case StoryMatchKind::OnboardingAyaFriendly:
            return "onboarding-aya";
        case StoryMatchKind::OnboardingEntry:
            return "onboarding-entry";
        case StoryMatchKind::Imagination1967:
            return "imagination-1967";
        case StoryMatchKind::TixLunch:
            return "tix-lunch";
        case StoryMatchKind::None:
        default:
            return "none";
    }
}

const char* story_intro_phase_name(const StoryIntroPhase phase) {
    switch (phase) {
        case StoryIntroPhase::Invite:
            return "Invite";
        case StoryIntroPhase::PlayMatch:
            return "Match";
        case StoryIntroPhase::BetweenBalls:
            return "Break";
        case StoryIntroPhase::NameEntry:
            return "Name";
        case StoryIntroPhase::RivalIntro:
            return "Intro";
        default:
            return "?";
    }
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

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
