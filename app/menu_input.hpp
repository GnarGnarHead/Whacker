#pragma once

#include "action_input.hpp"

namespace whacker::app {

constexpr int kNeutralKeyW = kKeyboardScancodeW;
constexpr int kNeutralKeyS = kKeyboardScancodeS;
constexpr int kNeutralKeyUp = kKeyboardScancodeUp;
constexpr int kNeutralKeyDown = kKeyboardScancodeDown;

struct ControlHintBindings {
    // Display-only movement key hints derived from ActionInputBindings.
    int p1_up = kNeutralKeyW;
    int p1_down = kNeutralKeyS;
    int p2_up = kNeutralKeyUp;
    int p2_down = kNeutralKeyDown;
};

}  // namespace whacker::app
