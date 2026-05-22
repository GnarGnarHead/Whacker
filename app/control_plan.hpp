#pragma once

#include "control_types.hpp"

namespace whacker::app {

constexpr CourtSide opposite_court_side(const CourtSide side) {
    return side == CourtSide::Left ? CourtSide::Right : CourtSide::Left;
}

constexpr MatchControlPlan quick_match_control_plan() {
    return MatchControlPlan {
        .p1_side = CourtSide::Left,
        .p2_side = CourtSide::Right,
    };
}

constexpr MatchControlPlan story_player_control_plan(const CourtSide player_side) {
    return MatchControlPlan {
        .p1_side = player_side,
        .p2_side = opposite_court_side(player_side),
    };
}

constexpr CourtSide court_side_for_input_slot(const MatchControlPlan& plan, const InputSlot slot) {
    return slot == InputSlot::P1 ? plan.p1_side : plan.p2_side;
}

constexpr InputSlot input_slot_for_court_side(const MatchControlPlan& plan, const CourtSide side) {
    return plan.p1_side == side ? InputSlot::P1 : InputSlot::P2;
}

constexpr float axis_for_input_slot(const InputSlotAxes& axes, const InputSlot slot) {
    return slot == InputSlot::P1 ? axes.p1_move_y : axes.p2_move_y;
}

constexpr float human_axis_for_court_side(
    const MatchControlPlan& plan,
    const InputSlotAxes& axes,
    const CourtSide side) {
    return axis_for_input_slot(axes, input_slot_for_court_side(plan, side));
}

}  // namespace whacker::app
