#pragma once

#include <cstdint>

#include <GL/gl.h>

namespace whacker::app {

struct RgbaTexture {
    GLuint texture_id = 0;
    int source_width = 0;
    int source_height = 0;
    int backing_width = 0;
    int backing_height = 0;
    float u_max = 1.0f;
    float v_max = 1.0f;
};

struct RgbaTextureUploadResult {
    bool uploaded = false;
    RgbaTexture texture {};
};

inline bool is_power_of_two_dimension(const int value) {
    return value > 0 && (value & (value - 1)) == 0;
}

inline int next_power_of_two_dimension(const int value) {
    if (value <= 0) {
        return 0;
    }
    int result = 1;
    while (result < value && result <= (1 << 29)) {
        result <<= 1;
    }
    return result >= value ? result : 0;
}

RgbaTextureUploadResult upload_rgba_texture(
    const std::uint8_t* rgba_pixels,
    int source_width,
    int source_height,
    const char* debug_label);
void release_rgba_texture(RgbaTexture& texture);
void draw_rgba_texture_quad_pixels(
    int fb_width,
    int fb_height,
    const RgbaTexture& texture,
    float x,
    float y,
    float w,
    float h,
    float alpha = 1.0f,
    float brightness = 1.0f,
    bool mirror_x = false,
    float rotation_deg = 0.0f);

}  // namespace whacker::app
