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

ControllerButton controller_button_from_sdl(const SDL_GameControllerButton button) {
    switch (button) {
        case SDL_CONTROLLER_BUTTON_A:
            return ControllerButton::A;
        case SDL_CONTROLLER_BUTTON_B:
            return ControllerButton::B;
        case SDL_CONTROLLER_BUTTON_X:
            return ControllerButton::X;
        case SDL_CONTROLLER_BUTTON_Y:
            return ControllerButton::Y;
        case SDL_CONTROLLER_BUTTON_BACK:
            return ControllerButton::Back;
        case SDL_CONTROLLER_BUTTON_GUIDE:
            return ControllerButton::Guide;
        case SDL_CONTROLLER_BUTTON_START:
            return ControllerButton::Start;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return ControllerButton::LeftStick;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return ControllerButton::RightStick;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return ControllerButton::LeftShoulder;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return ControllerButton::RightShoulder;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return ControllerButton::DpadUp;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return ControllerButton::DpadDown;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return ControllerButton::DpadLeft;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return ControllerButton::DpadRight;
        default:
            return ControllerButton::Unbound;
    }
}

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
    SDL_StartTextInput();
    should_close_ = false;
    return true;
}

void SdlPlatform::destroy_window() {
    if (gl_context_ != nullptr) {
        SDL_StopTextInput();
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
    event_frame_ = SdlEventFrame {};
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
            continue;
        }
        if (event.type == SDL_TEXTINPUT) {
            event_frame_.text_input += event.text.text;
            continue;
        }
        if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            event_frame_.keyboard_key_pressed = true;
            event_frame_.keyboard_scancode = static_cast<int>(event.key.keysym.scancode);
            event_frame_.keyboard_keycode = static_cast<int>(event.key.keysym.sym);
            if (event.key.keysym.sym == SDLK_BACKSPACE) {
                event_frame_.backspace_pressed = true;
            } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                event_frame_.text_confirm_pressed = true;
            }
            continue;
        }
        if (event.type == SDL_CONTROLLERBUTTONDOWN) {
            event_frame_.controller_button_pressed = true;
            event_frame_.controller_instance_id = static_cast<int>(event.cbutton.which);
            event_frame_.controller_index = -1;
            event_frame_.controller_button = controller_button_from_sdl(
                static_cast<SDL_GameControllerButton>(event.cbutton.button));
        }
    }
}

const SdlEventFrame& SdlPlatform::event_frame() const {
    return event_frame_;
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
    int width = 0;
    int height = 0;
    framebuffer_size(width, height);
    return make_letterboxed_render_context(width, height);
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
