#include "rgba_texture.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <vector>

#if defined(WHACKER_R36_GLES2_TEXTURES)
#include <dlfcn.h>
#endif

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

void set_enabled(const GLenum cap, const bool enabled) {
    if (enabled) {
        glEnable(cap);
    } else {
        glDisable(cap);
    }
}

#if defined(WHACKER_R36_GLES2_TEXTURES)

constexpr GLenum kGlTexture0 = 0x84C0;
constexpr GLenum kGlVertexShader = 0x8B31;
constexpr GLenum kGlFragmentShader = 0x8B30;
constexpr GLenum kGlCompileStatus = 0x8B81;
constexpr GLenum kGlLinkStatus = 0x8B82;

struct R36Gles2Api {
    void (*ActiveTexture)(GLenum texture) = nullptr;
    void (*AttachShader)(GLuint program, GLuint shader) = nullptr;
    void (*BindAttribLocation)(GLuint program, GLuint index, const char* name) = nullptr;
    void (*BindTexture)(GLenum target, GLuint texture) = nullptr;
    void (*CompileShader)(GLuint shader) = nullptr;
    GLuint (*CreateProgram)() = nullptr;
    GLuint (*CreateShader)(GLenum type) = nullptr;
    void (*DeleteProgram)(GLuint program) = nullptr;
    void (*DeleteShader)(GLuint shader) = nullptr;
    void (*DeleteTextures)(GLsizei n, const GLuint* textures) = nullptr;
    void (*DisableVertexAttribArray)(GLuint index) = nullptr;
    void (*DrawArrays)(GLenum mode, GLint first, GLsizei count) = nullptr;
    void (*EnableVertexAttribArray)(GLuint index) = nullptr;
    void (*GenTextures)(GLsizei n, GLuint* textures) = nullptr;
    GLint (*GetUniformLocation)(GLuint program, const char* name) = nullptr;
    void (*GetProgramiv)(GLuint program, GLenum pname, GLint* params) = nullptr;
    void (*GetShaderiv)(GLuint shader, GLenum pname, GLint* params) = nullptr;
    void (*LinkProgram)(GLuint program) = nullptr;
    void (*PixelStorei)(GLenum pname, GLint param) = nullptr;
    void (*ShaderSource)(GLuint shader, GLsizei count, const char* const* string, const GLint* length) = nullptr;
    void (*TexImage2D)(
        GLenum target,
        GLint level,
        GLint internalformat,
        GLsizei width,
        GLsizei height,
        GLint border,
        GLenum format,
        GLenum type,
        const void* pixels) = nullptr;
    void (*TexParameteri)(GLenum target, GLenum pname, GLint param) = nullptr;
    void (*Uniform1i)(GLint location, GLint value) = nullptr;
    void (*Uniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) = nullptr;
    void (*UseProgram)(GLuint program) = nullptr;
    void (*VertexAttribPointer)(
        GLuint index,
        GLint size,
        GLenum type,
        GLboolean normalized,
        GLsizei stride,
        const void* pointer) = nullptr;
};

void* r36_gles2_library() {
    static void* handle = [] {
        void* loaded = dlopen("libGLESv2.so", RTLD_NOW | RTLD_GLOBAL);
        if (loaded == nullptr) {
            loaded = dlopen("libGLESv2.so.2", RTLD_NOW | RTLD_GLOBAL);
        }
        return loaded;
    }();
    return handle;
}

template <typename Fn>
Fn r36_gles2_proc(const char* name) {
    void* library = r36_gles2_library();
    if (library == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<Fn>(dlsym(library, name));
}

R36Gles2Api& r36_gles2() {
    static R36Gles2Api api = [] {
        R36Gles2Api loaded {};
        loaded.ActiveTexture = r36_gles2_proc<void (*)(GLenum)>("glActiveTexture");
        loaded.AttachShader = r36_gles2_proc<void (*)(GLuint, GLuint)>("glAttachShader");
        loaded.BindAttribLocation = r36_gles2_proc<void (*)(GLuint, GLuint, const char*)>("glBindAttribLocation");
        loaded.BindTexture = r36_gles2_proc<void (*)(GLenum, GLuint)>("glBindTexture");
        loaded.CompileShader = r36_gles2_proc<void (*)(GLuint)>("glCompileShader");
        loaded.CreateProgram = r36_gles2_proc<GLuint (*)()>("glCreateProgram");
        loaded.CreateShader = r36_gles2_proc<GLuint (*)(GLenum)>("glCreateShader");
        loaded.DeleteProgram = r36_gles2_proc<void (*)(GLuint)>("glDeleteProgram");
        loaded.DeleteShader = r36_gles2_proc<void (*)(GLuint)>("glDeleteShader");
        loaded.DeleteTextures = r36_gles2_proc<void (*)(GLsizei, const GLuint*)>("glDeleteTextures");
        loaded.DisableVertexAttribArray = r36_gles2_proc<void (*)(GLuint)>("glDisableVertexAttribArray");
        loaded.DrawArrays = r36_gles2_proc<void (*)(GLenum, GLint, GLsizei)>("glDrawArrays");
        loaded.EnableVertexAttribArray = r36_gles2_proc<void (*)(GLuint)>("glEnableVertexAttribArray");
        loaded.GenTextures = r36_gles2_proc<void (*)(GLsizei, GLuint*)>("glGenTextures");
        loaded.GetUniformLocation = r36_gles2_proc<GLint (*)(GLuint, const char*)>("glGetUniformLocation");
        loaded.GetProgramiv = r36_gles2_proc<void (*)(GLuint, GLenum, GLint*)>("glGetProgramiv");
        loaded.GetShaderiv = r36_gles2_proc<void (*)(GLuint, GLenum, GLint*)>("glGetShaderiv");
        loaded.LinkProgram = r36_gles2_proc<void (*)(GLuint)>("glLinkProgram");
        loaded.PixelStorei = r36_gles2_proc<void (*)(GLenum, GLint)>("glPixelStorei");
        loaded.ShaderSource = r36_gles2_proc<void (*)(GLuint, GLsizei, const char* const*, const GLint*)>(
            "glShaderSource");
        loaded.TexImage2D = r36_gles2_proc<void (*)(
            GLenum,
            GLint,
            GLint,
            GLsizei,
            GLsizei,
            GLint,
            GLenum,
            GLenum,
            const void*)>("glTexImage2D");
        loaded.TexParameteri = r36_gles2_proc<void (*)(GLenum, GLenum, GLint)>("glTexParameteri");
        loaded.Uniform1i = r36_gles2_proc<void (*)(GLint, GLint)>("glUniform1i");
        loaded.Uniform4f = r36_gles2_proc<void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat)>("glUniform4f");
        loaded.UseProgram = r36_gles2_proc<void (*)(GLuint)>("glUseProgram");
        loaded.VertexAttribPointer = r36_gles2_proc<void (*)(
            GLuint,
            GLint,
            GLenum,
            GLboolean,
            GLsizei,
            const void*)>("glVertexAttribPointer");
        return loaded;
    }();
    return api;
}

bool r36_gles2_ready() {
    R36Gles2Api& gl = r36_gles2();
    return gl.ActiveTexture != nullptr &&
           gl.AttachShader != nullptr &&
           gl.BindAttribLocation != nullptr &&
           gl.BindTexture != nullptr &&
           gl.CompileShader != nullptr &&
           gl.CreateProgram != nullptr &&
           gl.CreateShader != nullptr &&
           gl.DeleteProgram != nullptr &&
           gl.DeleteShader != nullptr &&
           gl.DeleteTextures != nullptr &&
           gl.DisableVertexAttribArray != nullptr &&
           gl.DrawArrays != nullptr &&
           gl.EnableVertexAttribArray != nullptr &&
           gl.GenTextures != nullptr &&
           gl.GetUniformLocation != nullptr &&
           gl.GetProgramiv != nullptr &&
           gl.GetShaderiv != nullptr &&
           gl.LinkProgram != nullptr &&
           gl.PixelStorei != nullptr &&
           gl.ShaderSource != nullptr &&
           gl.TexImage2D != nullptr &&
           gl.TexParameteri != nullptr &&
           gl.Uniform1i != nullptr &&
           gl.Uniform4f != nullptr &&
           gl.UseProgram != nullptr &&
           gl.VertexAttribPointer != nullptr;
}

struct R36TextureProgram {
    bool attempted = false;
    bool ready = false;
    GLuint program = 0;
    GLint texture_uniform = -1;
    GLint color_uniform = -1;
};

R36TextureProgram& r36_texture_program() {
    static R36TextureProgram program {};
    return program;
}

GLuint r36_compile_shader(const GLenum type, const char* source) {
    R36Gles2Api& gl = r36_gles2();
    const GLuint shader = gl.CreateShader(type);
    if (shader == 0) {
        return 0;
    }
    gl.ShaderSource(shader, 1, &source, nullptr);
    gl.CompileShader(shader);

    GLint compiled = GL_FALSE;
    gl.GetShaderiv(shader, kGlCompileStatus, &compiled);
    if (compiled == GL_FALSE) {
        gl.DeleteShader(shader);
        return 0;
    }
    return shader;
}

bool ensure_r36_texture_program() {
    R36TextureProgram& state = r36_texture_program();
    if (state.ready) {
        return true;
    }
    if (state.attempted) {
        return false;
    }
    if (!r36_gles2_ready()) {
        std::fprintf(stderr, "Texture draw failed: GLES2 texture shader API unavailable\n");
        state.attempted = true;
        return false;
    }
    state.attempted = true;

    constexpr const char* kVertexShader = R"(
        attribute vec2 a_position;
        attribute vec2 a_texcoord;
        varying vec2 v_texcoord;
        void main() {
            gl_Position = vec4(a_position, 0.0, 1.0);
            v_texcoord = a_texcoord;
        }
    )";
    constexpr const char* kFragmentShader = R"(
        precision mediump float;
        uniform sampler2D u_texture;
        uniform vec4 u_color;
        varying vec2 v_texcoord;
        void main() {
            gl_FragColor = texture2D(u_texture, v_texcoord) * u_color;
        }
    )";

    R36Gles2Api& gl = r36_gles2();
    const GLuint vertex_shader = r36_compile_shader(kGlVertexShader, kVertexShader);
    if (vertex_shader == 0) {
        std::fprintf(stderr, "Texture draw failed: GLES2 vertex shader compilation failed\n");
        return false;
    }
    const GLuint fragment_shader = r36_compile_shader(kGlFragmentShader, kFragmentShader);
    if (fragment_shader == 0) {
        gl.DeleteShader(vertex_shader);
        std::fprintf(stderr, "Texture draw failed: GLES2 fragment shader compilation failed\n");
        return false;
    }

    const GLuint program = gl.CreateProgram();
    if (program == 0) {
        gl.DeleteShader(vertex_shader);
        gl.DeleteShader(fragment_shader);
        std::fprintf(stderr, "Texture draw failed: GLES2 shader program creation failed\n");
        return false;
    }

    gl.AttachShader(program, vertex_shader);
    gl.AttachShader(program, fragment_shader);
    gl.BindAttribLocation(program, 0, "a_position");
    gl.BindAttribLocation(program, 1, "a_texcoord");
    gl.LinkProgram(program);
    gl.DeleteShader(vertex_shader);
    gl.DeleteShader(fragment_shader);

    GLint linked = GL_FALSE;
    gl.GetProgramiv(program, kGlLinkStatus, &linked);
    if (linked == GL_FALSE) {
        gl.DeleteProgram(program);
        std::fprintf(stderr, "Texture draw failed: GLES2 shader program link failed\n");
        return false;
    }

    state.program = program;
    state.texture_uniform = gl.GetUniformLocation(program, "u_texture");
    state.color_uniform = gl.GetUniformLocation(program, "u_color");
    state.ready = state.texture_uniform >= 0 && state.color_uniform >= 0;
    if (!state.ready) {
        gl.DeleteProgram(program);
        state.program = 0;
        std::fprintf(stderr, "Texture draw failed: GLES2 shader uniforms unavailable\n");
    }
    return state.ready;
}

#endif

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

struct TextureVertex {
    GLfloat x = 0.0f;
    GLfloat y = 0.0f;
    GLfloat u = 0.0f;
    GLfloat v = 0.0f;
};

std::array<TextureVertex, 6> make_texture_vertices(
    const int fb_width,
    const int fb_height,
    const RgbaTexture& texture,
    const float x,
    const float y,
    const float w,
    const float h,
    const bool mirror_x,
    const float rotation_deg) {
    const float center_x = x + 0.5f * w;
    const float center_y = y + 0.5f * h;
    const float half_w = 0.5f * w;
    const float half_h = 0.5f * h;
    const float radians = rotation_deg * (3.1415926535f / 180.0f);
    const float cos_a = std::cos(radians);
    const float sin_a = std::sin(radians);
    const auto rotate_pixel_point = [center_x, center_y, cos_a, sin_a](const float local_x, const float local_y) {
        return std::array<float, 2> {
            center_x + (local_x * cos_a) - (local_y * sin_a),
            center_y + (local_x * sin_a) + (local_y * cos_a),
        };
    };
    const std::array<std::array<float, 2>, 4> corners_px {{
        rotate_pixel_point(-half_w, -half_h),
        rotate_pixel_point(half_w, -half_h),
        rotate_pixel_point(half_w, half_h),
        rotate_pixel_point(-half_w, half_h),
    }};
    const auto to_ndc_x = [fb_width](const float x_px) {
        return (x_px / static_cast<float>(fb_width)) * 2.0f - 1.0f;
    };
    const auto to_ndc_y = [fb_height](const float y_px) {
        return 1.0f - (y_px / static_cast<float>(fb_height)) * 2.0f;
    };
    const float u0 = mirror_x ? texture.u_max : 0.0f;
    const float u1 = mirror_x ? 0.0f : texture.u_max;
    return {{
        {to_ndc_x(corners_px[0][0]), to_ndc_y(corners_px[0][1]), u0, 0.0f},
        {to_ndc_x(corners_px[1][0]), to_ndc_y(corners_px[1][1]), u1, 0.0f},
        {to_ndc_x(corners_px[2][0]), to_ndc_y(corners_px[2][1]), u1, texture.v_max},
        {to_ndc_x(corners_px[0][0]), to_ndc_y(corners_px[0][1]), u0, 0.0f},
        {to_ndc_x(corners_px[2][0]), to_ndc_y(corners_px[2][1]), u1, texture.v_max},
        {to_ndc_x(corners_px[3][0]), to_ndc_y(corners_px[3][1]), u0, texture.v_max},
    }};
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
#if defined(WHACKER_R36_GLES2_TEXTURES)
    if (!r36_gles2_ready()) {
        std::fprintf(stderr, "Texture upload failed for %s: GLES2 texture API unavailable\n", safe_debug_label(debug_label));
        return result;
    }
    R36Gles2Api& r36_gl = r36_gles2();
    r36_gl.GenTextures(1, &texture_id);
#else
    glGenTextures(1, &texture_id);
#endif
    if (texture_id == 0) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment);
        std::fprintf(
            stderr,
            "Texture upload failed for %s: glGenTextures returned 0\n",
            safe_debug_label(debug_label));
        return result;
    }

#if defined(WHACKER_R36_GLES2_TEXTURES)
    r36_gl.ActiveTexture(kGlTexture0);
    r36_gl.BindTexture(GL_TEXTURE_2D, texture_id);
    r36_gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    r36_gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    r36_gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    r36_gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    r36_gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    r36_gl.TexImage2D(
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
    r36_gl.BindTexture(GL_TEXTURE_2D, 0);
    r36_gl.PixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment);
#else
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
#endif

    if (upload_error != GL_NO_ERROR) {
#if defined(WHACKER_R36_GLES2_TEXTURES)
        r36_gl.DeleteTextures(1, &texture_id);
#else
        glDeleteTextures(1, &texture_id);
#endif
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
#if defined(WHACKER_R36_GLES2_TEXTURES)
        if (r36_gles2().DeleteTextures != nullptr) {
            r36_gles2().DeleteTextures(1, &texture.texture_id);
        }
#else
        glDeleteTextures(1, &texture.texture_id);
#endif
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
    const bool mirror_x,
    const float rotation_deg) {
    const float safe_alpha = std::clamp(alpha, 0.0f, 1.0f);
    if (safe_alpha <= 0.0f || texture.texture_id == 0 || w <= 0.0f || h <= 0.0f || fb_width <= 0 || fb_height <= 0) {
        return;
    }

    const float safe_brightness = std::clamp(brightness, 0.0f, 2.0f);
    const std::array<TextureVertex, 6> vertices =
        make_texture_vertices(fb_width, fb_height, texture, x, y, w, h, mirror_x, rotation_deg);

    apply_full_pixel_scissor(fb_width, fb_height);
    const bool blend_was_enabled = (glIsEnabled(GL_BLEND) == GL_TRUE);
    const bool depth_was_enabled = (glIsEnabled(GL_DEPTH_TEST) == GL_TRUE);
    const bool stencil_was_enabled = (glIsEnabled(GL_STENCIL_TEST) == GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if defined(WHACKER_R36_GLES2_TEXTURES)
    if (ensure_r36_texture_program()) {
        R36Gles2Api& r36_gl = r36_gles2();
        R36TextureProgram& program = r36_texture_program();
        r36_gl.UseProgram(program.program);
        r36_gl.ActiveTexture(kGlTexture0);
        r36_gl.BindTexture(GL_TEXTURE_2D, texture.texture_id);
        r36_gl.Uniform1i(program.texture_uniform, 0);
        r36_gl.Uniform4f(program.color_uniform, safe_brightness, safe_brightness, safe_brightness, safe_alpha);
        r36_gl.EnableVertexAttribArray(0);
        r36_gl.EnableVertexAttribArray(1);
        r36_gl.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), &vertices[0].x);
        r36_gl.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), &vertices[0].u);
        r36_gl.DrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
        r36_gl.DisableVertexAttribArray(1);
        r36_gl.DisableVertexAttribArray(0);
        r36_gl.BindTexture(GL_TEXTURE_2D, 0);
        r36_gl.UseProgram(0);
    }
    set_enabled(GL_BLEND, blend_was_enabled);
    set_enabled(GL_STENCIL_TEST, stencil_was_enabled);
    set_enabled(GL_DEPTH_TEST, depth_was_enabled);
#else
    const auto to_pixel_x = [fb_width](const GLfloat ndc_x) {
        return (ndc_x + 1.0f) * 0.5f * static_cast<float>(fb_width);
    };
    const auto to_pixel_y = [fb_height](const GLfloat ndc_y) {
        return (1.0f - ndc_y) * 0.5f * static_cast<float>(fb_height);
    };
    const bool texture_was_enabled = (glIsEnabled(GL_TEXTURE_2D) == GL_TRUE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture.texture_id);
    glColor4f(safe_brightness, safe_brightness, safe_brightness, safe_alpha);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(fb_width), static_cast<double>(fb_height), 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_TRIANGLES);
    for (const TextureVertex& vertex : vertices) {
        glTexCoord2f(vertex.u, vertex.v);
        glVertex2f(to_pixel_x(vertex.x), to_pixel_y(vertex.y));
    }
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    set_enabled(GL_TEXTURE_2D, texture_was_enabled);
    set_enabled(GL_BLEND, blend_was_enabled);
    set_enabled(GL_STENCIL_TEST, stencil_was_enabled);
    set_enabled(GL_DEPTH_TEST, depth_was_enabled);
#endif
}

}  // namespace whacker::app
