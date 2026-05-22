#include "rgba_texture.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <vector>

#include "pixel_font.hpp"

namespace whacker::app {

namespace {

const char* safe_debug_label(const char* debug_label) {
    return debug_label != nullptr && debug_label[0] != '\0' ? debug_label : "rgba texture";
}

void drain_gl_errors() {
    while (glGetError() != GL_NO_ERROR) {
    }
}

std::vector<std::uint8_t> make_backing_rgba(
    const std::uint8_t* rgba_pixels,
    const int source_width,
    const int source_height,
    const int backing_width,
    const int backing_height) {
    const std::size_t source_row_bytes = static_cast<std::size_t>(source_width) * 4u;
    const std::size_t backing_row_bytes = static_cast<std::size_t>(backing_width) * 4u;
    std::vector<std::uint8_t> backing(
        static_cast<std::size_t>(backing_width) * static_cast<std::size_t>(backing_height) * 4u,
        0u);

    for (int y = 0; y < source_height; ++y) {
        const std::uint8_t* source_row = rgba_pixels + (static_cast<std::size_t>(y) * source_row_bytes);
        std::uint8_t* backing_row = backing.data() + (static_cast<std::size_t>(y) * backing_row_bytes);
        std::copy(source_row, source_row + source_row_bytes, backing_row);
    }
    return backing;
}

}  // namespace

RgbaTextureUploadResult upload_rgba_texture(
    const std::uint8_t* rgba_pixels,
    const int source_width,
    const int source_height,
    const char* debug_label) {
    RgbaTextureUploadResult result {};
    if (rgba_pixels == nullptr || source_width <= 0 || source_height <= 0) {
        std::fprintf(
            stderr,
            "Texture upload failed for %s: invalid RGBA input (%dx%d)\n",
            safe_debug_label(debug_label),
            source_width,
            source_height);
        return result;
    }

    const int backing_width = next_power_of_two_dimension(source_width);
    const int backing_height = next_power_of_two_dimension(source_height);
    if (backing_width <= 0 || backing_height <= 0) {
        std::fprintf(
            stderr,
            "Texture upload failed for %s: invalid POT backing size for %dx%d\n",
            safe_debug_label(debug_label),
            source_width,
            source_height);
        return result;
    }

    std::vector<std::uint8_t> backing;
    const std::uint8_t* upload_pixels = rgba_pixels;
    if (backing_width != source_width || backing_height != source_height) {
        backing = make_backing_rgba(rgba_pixels, source_width, source_height, backing_width, backing_height);
        upload_pixels = backing.data();
    }

    GLint previous_unpack_alignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previous_unpack_alignment);

    GLuint texture_id = 0;
    drain_gl_errors();
    glGenTextures(1, &texture_id);
    if (texture_id == 0) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment);
        std::fprintf(
            stderr,
            "Texture upload failed for %s: glGenTextures returned 0\n",
            safe_debug_label(debug_label));
        return result;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        backing_width,
        backing_height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        upload_pixels);
    const GLenum upload_error = glGetError();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment);

    if (upload_error != GL_NO_ERROR) {
        glDeleteTextures(1, &texture_id);
        std::fprintf(
            stderr,
            "Texture upload failed for %s: glGetError=0x%x source=%dx%d backing=%dx%d\n",
            safe_debug_label(debug_label),
            static_cast<unsigned>(upload_error),
            source_width,
            source_height,
            backing_width,
            backing_height);
        return result;
    }

    result.uploaded = true;
    result.texture = RgbaTexture {
        .texture_id = texture_id,
        .source_width = source_width,
        .source_height = source_height,
        .backing_width = backing_width,
        .backing_height = backing_height,
        .u_max = static_cast<float>(source_width) / static_cast<float>(backing_width),
        .v_max = static_cast<float>(source_height) / static_cast<float>(backing_height),
    };
    return result;
}

void release_rgba_texture(RgbaTexture& texture) {
    if (texture.texture_id != 0) {
        glDeleteTextures(1, &texture.texture_id);
    }
    texture = RgbaTexture {};
}

void draw_rgba_texture_quad_pixels(
    const int fb_width,
    const int fb_height,
    const RgbaTexture& texture,
    const float x,
    const float y,
    const float w,
    const float h,
    const float alpha,
    const float brightness,
    const bool mirror_x) {
    const float safe_alpha = std::clamp(alpha, 0.0f, 1.0f);
    if (safe_alpha <= 0.0f || texture.texture_id == 0 || w <= 0.0f || h <= 0.0f || fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const float safe_brightness = std::clamp(brightness, 0.0f, 2.0f);
    const float u0 = mirror_x ? texture.u_max : 0.0f;
    const float u1 = mirror_x ? 0.0f : texture.u_max;

    apply_full_pixel_scissor(fb_width, fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(fb_width), static_cast<double>(fb_height), 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture.texture_id);
    glColor4f(safe_brightness, safe_brightness, safe_brightness, safe_alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, 0.0f);
    glVertex2f(x, y);
    glTexCoord2f(u1, 0.0f);
    glVertex2f(x + w, y);
    glTexCoord2f(u1, texture.v_max);
    glVertex2f(x + w, y + h);
    glTexCoord2f(u0, texture.v_max);
    glVertex2f(x, y + h);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

}  // namespace whacker::app
