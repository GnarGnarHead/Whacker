#include "paddle_tuning_overlay.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "ai_style_catalog.hpp"
#include "pixel_font.hpp"

namespace whacker::app {

namespace {

struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;
};

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

void draw_triangle_edge(
    const int fb_width,
    const int fb_height,
    const float x0,
    const float y0,
    const float x1,
    const float y1,
    const Color color) {
    constexpr int kSamples = 96;
    for (int i = 0; i <= kSamples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kSamples);
        const float x = (1.0f - t) * x0 + (t * x1);
        const float y = (1.0f - t) * y0 + (t * y1);
        draw_rect_pixels(fb_width, fb_height, x - 1.0f, y - 1.0f, 2.0f, 2.0f, color.r, color.g, color.b);
    }
}

float orient2d(const Vec2f a, const Vec2f b, const Vec2f c) {
    return ((b.x - a.x) * (c.y - a.y)) - ((b.y - a.y) * (c.x - a.x));
}

bool point_in_triangle(const Vec2f p, const Vec2f a, const Vec2f b, const Vec2f c) {
    const float s1 = orient2d(a, b, p);
    const float s2 = orient2d(b, c, p);
    const float s3 = orient2d(c, a, p);
    const bool has_neg = (s1 < 0.0f) || (s2 < 0.0f) || (s3 < 0.0f);
    const bool has_pos = (s1 > 0.0f) || (s2 > 0.0f) || (s3 > 0.0f);
    return !(has_neg && has_pos);
}

void draw_filled_triangle(
    const int fb_width,
    const int fb_height,
    const Vec2f a,
    const Vec2f b,
    const Vec2f c,
    const Color color) {
    const float min_x = std::floor(std::min({a.x, b.x, c.x}));
    const float max_x = std::ceil(std::max({a.x, b.x, c.x}));
    const float min_y = std::floor(std::min({a.y, b.y, c.y}));
    const float max_y = std::ceil(std::max({a.y, b.y, c.y}));

    constexpr float kStep = 2.0f;
    for (float y = min_y; y <= max_y; y += kStep) {
        for (float x = min_x; x <= max_x; x += kStep) {
            const Vec2f sample {x + 1.0f, y + 1.0f};
            if (!point_in_triangle(sample, a, b, c)) {
                continue;
            }
            draw_rect_pixels(fb_width, fb_height, x, y, kStep, kStep, color.r, color.g, color.b);
        }
    }
}

Vec2f lerp(const Vec2f a, const Vec2f b, const float t_raw) {
    const float t = clampf(t_raw, 0.0f, 1.0f);
    return Vec2f {(a.x * (1.0f - t)) + (b.x * t), (a.y * (1.0f - t)) + (b.y * t)};
}

std::string percent_line_with_cap(
    const char* label,
    const float value_01,
    const float cap_01,
    const bool show_cap) {
    const int percent = static_cast<int>(std::lround(clampf(value_01, 0.0f, 1.0f) * 100.0f));
    std::string out = label;
    out += " ";
    out += std::to_string(percent);
    out += "%";
    if (!show_cap) {
        return out;
    }
    const int cap_percent = static_cast<int>(std::lround(clampf(cap_01, 0.0f, 1.0f) * 100.0f));
    out += " / ";
    out += std::to_string(cap_percent);
    out += "%";
    return out;
}

std::string format_fixed_2(const float value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f", static_cast<double>(value));
    return std::string(buffer);
}

void draw_stat_bar(
    const int fb_width,
    const int fb_height,
    const float x,
    const float y,
    const float w,
    const float h,
    const float value_01,
    const bool selected,
    const Color fill_color,
    const std::string& label) {
    const float outer_r = selected ? 0.28f : 0.16f;
    const float outer_g = selected ? 0.36f : 0.22f;
    const float outer_b = selected ? 0.50f : 0.30f;
    draw_rect_pixels(fb_width, fb_height, x, y, w, h, outer_r, outer_g, outer_b);
    draw_rect_pixels(fb_width, fb_height, x + 2.0f, y + 2.0f, w - 4.0f, h - 4.0f, 0.10f, 0.14f, 0.20f);
    const float fill_w = (w - 8.0f) * clampf(value_01, 0.0f, 1.0f);
    draw_rect_pixels(fb_width, fb_height, x + 4.0f, y + 4.0f, fill_w, h - 8.0f, fill_color.r, fill_color.g, fill_color.b);
    draw_text_pixels(
        fb_width,
        fb_height,
        x + 8.0f,
        y + 7.0f,
        1.8f,
        label,
        Color {0.92f, 0.96f, 1.00f});
}

}  // namespace

void render_paddle_tuning_overlay(const RenderContext& context, const PaddleTuningState& tuning_state) {
    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const PaddleTuning tuning = tuning_state.working;
    const whacker::progression::SkillState skills = paddle_tuning_to_skills(tuning);
    const bool show_limits = tuning_state.target == PaddleTuningTarget::StoryPlayer;
    const whacker::progression::SkillState limit_skills = tuning_state.max_skills;
    const float budget_limit = show_limits ? tuning_state.max_budget : kPaddleTuningBudgetCap;
    const AiStyle style = style_for_skills(skills);

    const float panel_w = static_cast<float>(fb_width) * 0.62f;
    const float panel_h = static_cast<float>(fb_height) * 0.72f;
    const float panel_x = 0.5f * (static_cast<float>(fb_width) - panel_w);
    const float panel_y = 0.5f * (static_cast<float>(fb_height) - panel_h);
    draw_rect_pixels(fb_width, fb_height, panel_x, panel_y, panel_w, panel_h, 0.05f, 0.09f, 0.14f);
    draw_rect_pixels(fb_width, fb_height, panel_x + 4.0f, panel_y + 4.0f, panel_w - 8.0f, 44.0f, 0.09f, 0.16f, 0.24f);

    draw_text_pixels(
        fb_width,
        fb_height,
        panel_x + 16.0f,
        panel_y + 16.0f,
        2.6f,
        paddle_tuning_target_title(tuning_state.target),
        Color {0.90f, 0.94f, 1.00f});

    const float tri_center_x = panel_x + (panel_w * 0.33f);
    const float tri_center_y = panel_y + (panel_h * 0.52f);
    const float tri_scale = std::min(panel_w * 0.20f, panel_h * 0.32f);

    const float edge_x = tri_center_x + (0.0f * tri_scale);
    const float edge_y = tri_center_y - (1.0f * tri_scale);
    const float power_x = tri_center_x + (-0.8660254f * tri_scale);
    const float power_y = tri_center_y - (-0.5f * tri_scale);
    const float spin_x = tri_center_x + (0.8660254f * tri_scale);
    const float spin_y = tri_center_y - (-0.5f * tri_scale);

    const Vec2f edge_v {edge_x, edge_y};
    const Vec2f power_v {power_x, power_y};
    const Vec2f spin_v {spin_x, spin_y};
    const Vec2f centroid {
        (edge_v.x + power_v.x + spin_v.x) / 3.0f,
        (edge_v.y + power_v.y + spin_v.y) / 3.0f};
    const Vec2f fill_edge = lerp(centroid, edge_v, skills.edge);
    const Vec2f fill_power = lerp(centroid, power_v, skills.power);
    const Vec2f fill_spin = lerp(centroid, spin_v, skills.spin_inject);
    const Color fill_color {
        0.18f + (0.26f * clampf(skills.power, 0.0f, 1.0f)),
        0.18f + (0.26f * clampf(skills.edge, 0.0f, 1.0f)),
        0.18f + (0.26f * clampf(skills.spin_inject, 0.0f, 1.0f))};
    draw_filled_triangle(fb_width, fb_height, fill_edge, fill_power, fill_spin, fill_color);

    draw_triangle_edge(fb_width, fb_height, edge_x, edge_y, power_x, power_y, Color {0.50f, 0.86f, 0.58f});
    draw_triangle_edge(fb_width, fb_height, power_x, power_y, spin_x, spin_y, Color {0.94f, 0.58f, 0.40f});
    draw_triangle_edge(fb_width, fb_height, spin_x, spin_y, edge_x, edge_y, Color {0.40f, 0.78f, 0.98f});

    draw_text_pixels(fb_width, fb_height, edge_x - 18.0f, edge_y - 20.0f, 1.9f, "TEC", Color {0.42f, 0.90f, 0.56f});
    draw_text_pixels(fb_width, fb_height, power_x - 28.0f, power_y + 10.0f, 1.9f, "POW", Color {0.96f, 0.55f, 0.38f});
    draw_text_pixels(fb_width, fb_height, spin_x - 16.0f, spin_y + 10.0f, 1.9f, "SPN", Color {0.38f, 0.76f, 0.98f});

    const float info_x = panel_x + (panel_w * 0.58f);
    draw_text_pixels(
        fb_width,
        fb_height,
        info_x,
        panel_y + 94.0f,
        2.2f,
        std::string("STYLE  ") + whacker::app::ai_style_display_name(style),
        Color {0.88f, 0.93f, 1.00f});
    draw_stat_bar(
        fb_width,
        fb_height,
        info_x,
        panel_y + 122.0f,
        panel_w * 0.34f,
        24.0f,
        skills.edge,
        tuning_state.selected_component == 0,
        Color {0.44f, 0.90f, 0.58f},
        percent_line_with_cap("TEC", skills.edge, limit_skills.edge, show_limits));
    draw_stat_bar(
        fb_width,
        fb_height,
        info_x,
        panel_y + 152.0f,
        panel_w * 0.34f,
        24.0f,
        skills.power,
        tuning_state.selected_component == 1,
        Color {0.96f, 0.56f, 0.40f},
        percent_line_with_cap("POW", skills.power, limit_skills.power, show_limits));
    draw_stat_bar(
        fb_width,
        fb_height,
        info_x,
        panel_y + 182.0f,
        panel_w * 0.34f,
        24.0f,
        skills.spin_inject,
        tuning_state.selected_component == 2,
        Color {0.40f, 0.78f, 0.98f},
        percent_line_with_cap("SPN", skills.spin_inject, limit_skills.spin_inject, show_limits));
    const float total = skills.edge + skills.power + skills.spin_inject;
    draw_text_pixels(
        fb_width,
        fb_height,
        info_x,
        panel_y + 220.0f,
        1.9f,
        "TOTAL " + format_fixed_2(total) + " / " + format_fixed_2(budget_limit),
        Color {0.76f, 0.84f, 0.92f});

    draw_text_pixels(
        fb_width,
        fb_height,
        panel_x + 16.0f,
        panel_y + panel_h - 24.0f,
        1.8f,
        "UP/DOWN BAR  LEFT/RIGHT ADJUST  ENTER APPLY  ESC CANCEL",
        Color {0.70f, 0.78f, 0.88f});
}

}  // namespace whacker::app
