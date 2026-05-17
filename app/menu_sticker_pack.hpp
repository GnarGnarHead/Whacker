#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace whacker::app {

enum class MenuStickerSurface : std::uint8_t {
    MainMenu = 0,
    MainMenuBack = 1,
    StoryMenu = 2,
    StoryHub = 3,
    QuickSetup = 4,
    OptionsMenu = 5,
    PauseMenu = 6,
};

enum class MenuStickerAnchor : std::uint8_t {
    TopLeft = 0,
    TopRight = 1,
    BottomLeft = 2,
    BottomRight = 3,
    LeftEdge = 4,
    RightEdge = 5,
};

enum class MenuStickerPlacement : std::uint8_t {
    Edge = 0,
    Inside = 1,
};

enum class MenuStickerSizeMode : std::uint8_t {
    Scale = 0,
    HeightPx = 1,
};

struct MenuStickerSlotSpec {
    std::string_view asset_filename {};
    MenuStickerAnchor anchor = MenuStickerAnchor::TopLeft;
    MenuStickerPlacement placement = MenuStickerPlacement::Edge;
    float x_norm = 0.5f;
    float y_norm = 0.5f;
    MenuStickerSizeMode size_mode = MenuStickerSizeMode::Scale;
    float scale = 0.20f;
    float height_px = 96.0f;
    float offset_x_px = 0.0f;
    float offset_y_px = 0.0f;
    float rotation_deg = 0.0f;
    bool allow_protected_overlap = false;
    int z_order = 0;
};

struct MenuStickerSurfaceSpec {
    MenuStickerSurface surface = MenuStickerSurface::MainMenu;
    std::span<const MenuStickerSlotSpec> slots {};
};

}  // namespace whacker::app
