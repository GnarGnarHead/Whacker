#pragma once

#include <span>

#include "menu_sticker_pack.hpp"

namespace whacker::app {

struct MenuStickerRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

void render_menu_stickers(
    int fb_width,
    int fb_height,
    MenuStickerSurface surface,
    const MenuStickerRect& panel_rect,
    std::span<const MenuStickerRect> protected_regions);

}  // namespace whacker::app

