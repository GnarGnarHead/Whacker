#pragma once

#include "action_input.hpp"

namespace whacker::app {

const char* main_menu_row_name(int row);
const char* options_menu_row_name(int row);
const char* story_menu_row_name(int row);
const char* story_hub_row_name(int row);
const char* controller_button_label(ControllerButton button);
const char* controller_axis_label(ControllerAxis axis);
const char* sdl_key_name(int key);

}  // namespace whacker::app
