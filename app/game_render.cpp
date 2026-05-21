#include "game_render.hpp"

#include <algorithm>
#include <string>

#include <GL/gl.h>

#include "pixel_font.hpp"

namespace {

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

}  // namespace

namespace whacker::app {

void render_scene(const RenderContext& context, const whacker::sim::Simulation& simulation, const bool ball_visible) {
    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const auto& config = simulation.config();
    const auto& state = simulation.state();
    const float sx = static_cast<float>(fb_width) / config.court_width;
    const float sy = static_cast<float>(fb_height) / config.court_height;

    glViewport(0, 0, fb_width, fb_height);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    draw_rect_pixels(
        fb_width,
        fb_height,
        0.0f,
        0.0f,
        static_cast<float>(fb_width),
        static_cast<float>(fb_height),
        0.05f,
        0.08f,
        0.11f);

    const float left_x = config.paddle_x_margin - config.paddle_half_width;
    const float right_x = config.court_width - config.paddle_x_margin - config.paddle_half_width;
    const float paddle_w = config.paddle_half_width * 2.0f;
    const float paddle_h = config.paddle_half_height * 2.0f;
    draw_rect_pixels(
        fb_width,
        fb_height,
        left_x * sx,
        (state.left.center_y - config.paddle_half_height) * sy,
        paddle_w * sx,
        paddle_h * sy,
        0.90f,
        0.93f,
        0.98f);
    draw_rect_pixels(
        fb_width,
        fb_height,
        right_x * sx,
        (state.right.center_y - config.paddle_half_height) * sy,
        paddle_w * sx,
        paddle_h * sy,
        0.90f,
        0.93f,
        0.98f);

    if (ball_visible) {
        const float ball_size = config.ball_radius * 2.0f;
        draw_rect_pixels(
            fb_width,
            fb_height,
            (state.ball.position.x - config.ball_radius) * sx,
            (state.ball.position.y - config.ball_radius) * sy,
            ball_size * sx,
            ball_size * sy,
            0.98f,
            0.50f,
            0.22f);
    }

    const float center_line_x = (config.court_width * 0.5f - 1.0f) * sx;
    draw_rect_pixels(
        fb_width,
        fb_height,
        center_line_x,
        0.0f,
        2.0f,
        static_cast<float>(fb_height),
        0.14f,
        0.20f,
        0.26f);
}

void render_hud(const RenderContext& context, const whacker::sim::Simulation& simulation) {
    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const auto& state = simulation.state();

    const float score_scale = 2.0f;
    const float score_block_w = 22.0f * score_scale;
    const float center_x = 0.5f * static_cast<float>(fb_width);
    const float score_gap = 82.0f;
    const float left_score_x = center_x - score_gap - score_block_w;
    const float right_score_x = center_x + score_gap;
    const float score_y = 10.0f;
    draw_two_digits(
        fb_width,
        fb_height,
        left_score_x,
        score_y,
        score_scale,
        state.left_score,
        Color {0.90f, 0.94f, 1.00f});
    draw_two_digits(
        fb_width,
        fb_height,
        right_score_x,
        score_y,
        score_scale,
        state.right_score,
        Color {0.90f, 0.94f, 1.00f});
}

void render_play_center_message(const RenderContext& context, const std::string& message) {
    if (message.empty()) {
        return;
    }

    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    constexpr float kScale = 2.0f;
    const float panel_w = std::clamp(static_cast<float>(fb_width) * 0.66f, 320.0f, 860.0f);
    const float panel_h = 32.0f;
    const float panel_x = 0.5f * (static_cast<float>(fb_width) - panel_w);
    const float panel_y = std::max(56.0f, static_cast<float>(fb_height) * 0.18f);
    draw_rect_pixels(fb_width, fb_height, panel_x, panel_y, panel_w, panel_h, 0.08f, 0.12f, 0.18f);
    draw_rect_pixels(fb_width, fb_height, panel_x + 2.0f, panel_y + 2.0f, panel_w - 4.0f, panel_h - 4.0f, 0.14f, 0.20f, 0.29f);
    draw_text_centered(
        fb_width,
        fb_height,
        panel_x + 8.0f,
        panel_y,
        panel_w - 16.0f,
        panel_h,
        kScale,
        message,
        Color {0.94f, 0.96f, 1.0f});
}

void render_dev_overlay(
    const RenderContext& context,
    const whacker::sim::Simulation& simulation,
    const bool ai_controls_player_paddle) {
    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const auto& config = simulation.config();
    const auto& state = simulation.state();

    const float panel_w = std::min(340.0f, std::max(260.0f, static_cast<float>(fb_width) * 0.34f));
    const float panel_h = 88.0f;
    const float panel_x = static_cast<float>(fb_width) - panel_w - 10.0f;
    const float panel_y = 52.0f;

    draw_rect_pixels(fb_width, fb_height, panel_x, panel_y, panel_w, panel_h, 0.06f, 0.10f, 0.15f);
    draw_rect_pixels(fb_width, fb_height, panel_x + 2.0f, panel_y + 2.0f, panel_w - 4.0f, panel_h - 4.0f, 0.10f, 0.15f, 0.23f);
    draw_text_pixels(fb_width, fb_height, panel_x + 10.0f, panel_y + 8.0f, 1.5f, "DEV INFO (F10)", Color {0.82f, 0.90f, 0.98f});
    draw_text_pixels(
        fb_width,
        fb_height,
        panel_x + 10.0f,
        panel_y + 24.0f,
        1.0f,
        ai_controls_player_paddle ? "PLAYER AI: ON (P)" : "PLAYER AI: OFF (P)",
        Color {0.76f, 0.86f, 0.94f});

    const float meter_label_scale = 1.5f;
    const float meter_x = panel_x + 68.0f;
    const float meter_w = panel_w - 82.0f;
    const float speed_y = panel_y + 44.0f;
    const float spin_y = panel_y + 62.0f;

    draw_text_pixels(fb_width, fb_height, panel_x + 10.0f, speed_y + 1.0f, meter_label_scale, "SPD", Color {0.82f, 0.88f, 0.95f});
    draw_rect_pixels(fb_width, fb_height, meter_x, speed_y, meter_w, 8.0f, 0.14f, 0.18f, 0.22f);
    const float speed_ratio = clampf((state.ball.speed_scalar - 1.0f) / 4.0f, 0.0f, 1.0f);
    draw_rect_pixels(fb_width, fb_height, meter_x, speed_y, meter_w * speed_ratio, 8.0f, 0.24f, 0.76f, 0.36f);

    draw_text_pixels(fb_width, fb_height, panel_x + 10.0f, spin_y + 1.0f, meter_label_scale, "SPN", Color {0.82f, 0.88f, 0.95f});
    draw_rect_pixels(fb_width, fb_height, meter_x, spin_y, meter_w, 8.0f, 0.14f, 0.18f, 0.22f);
    const float spin_center_x = meter_x + (0.5f * meter_w);
    draw_rect_pixels(fb_width, fb_height, spin_center_x - 1.0f, spin_y, 2.0f, 8.0f, 0.28f, 0.33f, 0.39f);
    const float spin_denom = std::max(config.spin_max, 1.0e-4f);
    const float spin_ratio = clampf(state.ball.spin / spin_denom, -1.0f, 1.0f);
    const float half_w = 0.5f * meter_w;
    if (spin_ratio > 0.0f) {
        draw_rect_pixels(fb_width, fb_height, spin_center_x, spin_y, half_w * spin_ratio, 8.0f, 0.96f, 0.52f, 0.20f);
    } else if (spin_ratio < 0.0f) {
        const float w = half_w * -spin_ratio;
        draw_rect_pixels(fb_width, fb_height, spin_center_x - w, spin_y, w, 8.0f, 0.28f, 0.68f, 0.98f);
    }
}

}  // namespace whacker::app
