#pragma once

#include "ui_state.hpp"

namespace whacker::app {

class SdlPlatform;

using MainMenuRowNameFn = const char* (*)(int);

void render_main_menu_overlay(
    SdlPlatform* platform,
    const MainMenuState& menu_state,
    MainMenuRowNameFn row_name_fn);

}  // namespace whacker::app
