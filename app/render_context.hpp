#pragma once

namespace whacker::app {

struct RenderContext {
    int framebuffer_width = 0;
    int framebuffer_height = 0;
};

inline bool render_context_valid(const RenderContext& context) {
    return context.framebuffer_width > 0 && context.framebuffer_height > 0;
}

}  // namespace whacker::app
