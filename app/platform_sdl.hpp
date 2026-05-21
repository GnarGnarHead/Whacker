#pragma once

#include <string>

#include "render_context.hpp"

struct SDL_Window;

namespace whacker::app {

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
};

}  // namespace whacker::app
