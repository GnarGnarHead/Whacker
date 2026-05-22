#include "render_context.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << "render_context_smoke failed: " << message << "\n";
        std::exit(1);
    }
}

void test_native_16_by_9_uses_full_drawable() {
    const whacker::app::RenderContext context =
        whacker::app::make_letterboxed_render_context(1920, 1080);

    require(context.drawable_width == 1920, "native drawable width preserved");
    require(context.drawable_height == 1080, "native drawable height preserved");
    require(context.framebuffer_width == 1920, "native content width");
    require(context.framebuffer_height == 1080, "native content height");
    require(context.viewport_x == 0, "native viewport x");
    require(context.viewport_y == 0, "native viewport y");
}

void test_tall_drawable_letterboxes_vertically() {
    const whacker::app::RenderContext context =
        whacker::app::make_letterboxed_render_context(1280, 1024);

    require(context.framebuffer_width == 1280, "tall content width");
    require(context.framebuffer_height == 720, "tall content height");
    require(context.viewport_x == 0, "tall viewport x");
    require(context.viewport_y == 152, "tall viewport y");
}

void test_wide_drawable_pillarboxes_horizontally() {
    const whacker::app::RenderContext context =
        whacker::app::make_letterboxed_render_context(2000, 900);

    require(context.framebuffer_width == 1600, "wide content width");
    require(context.framebuffer_height == 900, "wide content height");
    require(context.viewport_x == 200, "wide viewport x");
    require(context.viewport_y == 0, "wide viewport y");
}

void test_invalid_drawable_is_invalid_context() {
    const whacker::app::RenderContext context =
        whacker::app::make_letterboxed_render_context(0, 720);

    require(!whacker::app::render_context_valid(context), "invalid drawable context");
    require(context.drawable_width == 0, "invalid drawable width preserved");
    require(context.drawable_height == 720, "invalid drawable height preserved");
}

}  // namespace

int main() {
    test_native_16_by_9_uses_full_drawable();
    test_tall_drawable_letterboxes_vertically();
    test_wide_drawable_pillarboxes_horizontally();
    test_invalid_drawable_is_invalid_context();
    return 0;
}
