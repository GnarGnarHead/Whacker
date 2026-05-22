#pragma once

namespace whacker::app {

struct RenderContext {
    // Aspect-locked content area, in drawable pixels. Existing render code
    // consumes these as its logical framebuffer dimensions.
    int framebuffer_width = 0;
    int framebuffer_height = 0;
    int drawable_width = 0;
    int drawable_height = 0;
    int viewport_x = 0;
    int viewport_y = 0;
};

inline bool render_context_valid(const RenderContext& context) {
    return context.framebuffer_width > 0 && context.framebuffer_height > 0;
}

inline RenderContext make_letterboxed_render_context(const int drawable_width, const int drawable_height) {
    RenderContext context {};
    context.drawable_width = drawable_width;
    context.drawable_height = drawable_height;
    if (drawable_width <= 0 || drawable_height <= 0) {
        return context;
    }

    constexpr int kAspectWidth = 16;
    constexpr int kAspectHeight = 9;
    const int width_for_height = (drawable_height * kAspectWidth) / kAspectHeight;
    if (width_for_height <= drawable_width) {
        context.framebuffer_width = width_for_height;
        context.framebuffer_height = drawable_height;
    } else {
        context.framebuffer_width = drawable_width;
        context.framebuffer_height = (drawable_width * kAspectHeight) / kAspectWidth;
    }
    context.viewport_x = (drawable_width - context.framebuffer_width) / 2;
    context.viewport_y = (drawable_height - context.framebuffer_height) / 2;
    return context;
}

}  // namespace whacker::app
