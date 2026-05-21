#include "platform_sdl.hpp"

#ifndef WHACKER_PLATFORM_SDL2
#error "platform_sdl.cpp must be compiled with WHACKER_PLATFORM_SDL2"
#endif

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

namespace whacker::app {

namespace {

constexpr Uint32 kOwnedSubsystems =
    SDL_INIT_VIDEO |
    SDL_INIT_EVENTS |
    SDL_INIT_TIMER |
    SDL_INIT_GAMECONTROLLER;

}  // namespace

SdlPlatform::~SdlPlatform() {
    shutdown();
}

bool SdlPlatform::init(std::string* error_message) {
    if (initialized_) {
        return true;
    }

    SDL_SetMainReady();
#ifdef SDL_HINT_APP_NAME
    (void)SDL_SetHint(SDL_HINT_APP_NAME, "Whacker");
#endif

    if (SDL_Init(kOwnedSubsystems) != 0) {
        if (error_message != nullptr) {
            *error_message = SDL_GetError();
        }
        return false;
    }

    initialized_ = true;
    return true;
}

bool SdlPlatform::create_window(
    const int width,
    const int height,
    const char* title,
    std::string* error_message) {
    if (!initialized_ && !init(error_message)) {
        return false;
    }
    destroy_window();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    constexpr Uint32 kWindowFlags =
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_ALLOW_HIGHDPI;
    window_ = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        kWindowFlags);
    if (window_ == nullptr) {
        if (error_message != nullptr) {
            *error_message = SDL_GetError();
        }
        return false;
    }

    gl_context_ = SDL_GL_CreateContext(window_);
    if (gl_context_ == nullptr) {
        if (error_message != nullptr) {
            *error_message = SDL_GetError();
        }
        destroy_window();
        return false;
    }
    if (SDL_GL_MakeCurrent(window_, gl_context_) != 0) {
        if (error_message != nullptr) {
            *error_message = SDL_GetError();
        }
        destroy_window();
        return false;
    }

    (void)SDL_GL_SetSwapInterval(1);
    should_close_ = false;
    return true;
}

void SdlPlatform::destroy_window() {
    if (gl_context_ != nullptr) {
        SDL_GL_DeleteContext(gl_context_);
        gl_context_ = nullptr;
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    should_close_ = false;
}

void SdlPlatform::poll_events() {
    SDL_Event event {};
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            should_close_ = true;
            continue;
        }
        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
            window_ != nullptr &&
            event.window.windowID == SDL_GetWindowID(window_)) {
            should_close_ = true;
        }
    }
}

void SdlPlatform::request_close() {
    should_close_ = true;
}

void SdlPlatform::swap_buffers() {
    if (window_ != nullptr) {
        SDL_GL_SwapWindow(window_);
    }
}

void SdlPlatform::framebuffer_size(int& width, int& height) const {
    width = 0;
    height = 0;
    if (window_ != nullptr) {
        SDL_GL_GetDrawableSize(window_, &width, &height);
    }
}

RenderContext SdlPlatform::render_context() const {
    RenderContext context {};
    framebuffer_size(context.framebuffer_width, context.framebuffer_height);
    return context;
}

void SdlPlatform::set_window_title(const char* title) {
    if (window_ != nullptr) {
        SDL_SetWindowTitle(window_, title);
    }
}

bool SdlPlatform::should_close() const {
    return should_close_;
}

void SdlPlatform::shutdown() {
    if (!initialized_) {
        return;
    }
    destroy_window();
    SDL_QuitSubSystem(kOwnedSubsystems);
    initialized_ = false;
}

bool SdlPlatform::initialized() const {
    return initialized_;
}

double SdlPlatform::now_seconds() const {
    return static_cast<double>(SDL_GetTicks64()) / 1000.0;
}

}  // namespace whacker::app
