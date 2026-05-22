#pragma once

#include <cstdint>

namespace whacker::app {

enum class InputSlot : std::uint8_t {
    P1 = 0,
    P2 = 1
};

enum class CourtSide : std::uint8_t {
    Left = 0,
    Right = 1
};

struct MenuIntent {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool confirm = false;
    bool back = false;
};

struct MatchControlPlan {
    CourtSide p1_side = CourtSide::Left;
    CourtSide p2_side = CourtSide::Right;
};

struct InputSlotAxes {
    float p1_move_y = 0.0f;
    float p2_move_y = 0.0f;
};

}  // namespace whacker::app
