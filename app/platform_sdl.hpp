#pragma once

#include <string>

#include "action_input.hpp"
#include "render_context.hpp"

struct SDL_Window;

namespace whacker::app {

struct SdlEventFrame {
    std::string text_input {};
    bool backspace_pressed = false;
    bool text_confirm_pressed = false;
    bool keyboard_key_pressed = false;
    int keyboard_scancode = -1;
    int keyboard_keycode = 0;
    bool controller_button_pressed = false;
    int controller_instance_id = -1;
    int controller_index = 0;
    ControllerButton controller_button = ControllerButton::Unbound;
};

class SdlPlatform {
public:
    SdlPlatform() = default;
    ~SdlPlatform();

    SdlPlatform(const SdlPlatform&) = delete;
    SdlPlatform& operator=(const SdlPlatform&) = delete;

    bool init(std::string* error_message = nullptr);
    void shutdown();

    bool create_window(int width, int height, const char* title, std::string* error_message = nullptr);
    void destroy_window();
    void poll_events();
    const SdlEventFrame& event_frame() const;
    void request_close();
    void swap_buffers();
    void framebuffer_size(int& width, int& height) const;
    RenderContext render_context() const;
    void set_window_title(const char* title);
    bool should_close() const;
    bool initialized() const;
    double now_seconds() const;

private:
    SDL_Window* window_ = nullptr;
    void* gl_context_ = nullptr;
    bool initialized_ = false;
    bool should_close_ = false;
    SdlEventFrame event_frame_ {};
};

}  // namespace whacker::app
