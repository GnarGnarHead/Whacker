#pragma once

#include <cstdint>
#include <string>

#include "app_types.hpp"

namespace whacker::app {

void draw_rect_pixels(
    int fb_width,
    int fb_height,
    float x,
    float y,
    float w,
    float h,
    float r,
    float g,
    float b);

void draw_text_pixels(
    int fb_width,
    int fb_height,
    float x,
    float y,
    float scale,
    const std::string& text,
    Color color);

void draw_text_centered(
    int fb_width,
    int fb_height,
    float x,
    float y,
    float w,
    float h,
    float scale,
    const std::string& text,
    Color color);

float text_char_advance_pixels(float scale);
float text_line_height_pixels(float scale);

void draw_two_digits(
    int fb_width,
    int fb_height,
    float x,
    float y,
    float scale,
    int value,
    Color color);

void draw_rgba_sprite_pixels(
    int fb_width,
    int fb_height,
    float x,
    float y,
    float w,
    float h,
    int sprite_width,
    int sprite_height,
    const std::uint8_t* rgba_pixels,
    float alpha = 1.0f,
    float brightness = 1.0f,
    bool mirror_x = false);

}  // namespace whacker::app
