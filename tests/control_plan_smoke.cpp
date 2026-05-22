#include "control_plan.hpp"
#include "test_assert.hpp"

namespace {

void test_quick_match_plan_keeps_desktop_slots_on_default_sides() {
    constexpr whacker::app::MatchControlPlan plan = whacker::app::quick_match_control_plan();

    static_assert(plan.p1_side == whacker::app::CourtSide::Left);
    static_assert(plan.p2_side == whacker::app::CourtSide::Right);
    TEST_CHECK(whacker::app::court_side_for_input_slot(plan, whacker::app::InputSlot::P1) ==
               whacker::app::CourtSide::Left);
    TEST_CHECK(whacker::app::court_side_for_input_slot(plan, whacker::app::InputSlot::P2) ==
               whacker::app::CourtSide::Right);
}

void test_story_player_plan_can_put_player_on_either_side() {
    constexpr whacker::app::MatchControlPlan player_left =
        whacker::app::story_player_control_plan(whacker::app::CourtSide::Left);
    constexpr whacker::app::MatchControlPlan player_right =
        whacker::app::story_player_control_plan(whacker::app::CourtSide::Right);

    static_assert(player_left.p1_side == whacker::app::CourtSide::Left);
    static_assert(player_left.p2_side == whacker::app::CourtSide::Right);
    static_assert(player_right.p1_side == whacker::app::CourtSide::Right);
    static_assert(player_right.p2_side == whacker::app::CourtSide::Left);
    TEST_CHECK(whacker::app::opposite_court_side(whacker::app::CourtSide::Left) ==
               whacker::app::CourtSide::Right);
    TEST_CHECK(whacker::app::opposite_court_side(whacker::app::CourtSide::Right) ==
               whacker::app::CourtSide::Left);
}

void test_slot_and_side_lookup_round_trip_valid_plans() {
    constexpr whacker::app::MatchControlPlan plan =
        whacker::app::story_player_control_plan(whacker::app::CourtSide::Right);

    const whacker::app::CourtSide p1_side =
        whacker::app::court_side_for_input_slot(plan, whacker::app::InputSlot::P1);
    const whacker::app::CourtSide p2_side =
        whacker::app::court_side_for_input_slot(plan, whacker::app::InputSlot::P2);

    TEST_CHECK(whacker::app::input_slot_for_court_side(plan, p1_side) == whacker::app::InputSlot::P1);
    TEST_CHECK(whacker::app::input_slot_for_court_side(plan, p2_side) == whacker::app::InputSlot::P2);
}

void test_human_axis_lookup_uses_plan_slots() {
    constexpr whacker::app::InputSlotAxes axes {
        .p1_move_y = -0.5f,
        .p2_move_y = 0.75f,
    };
    constexpr whacker::app::MatchControlPlan quick_plan = whacker::app::quick_match_control_plan();
    constexpr whacker::app::MatchControlPlan story_right_plan =
        whacker::app::story_player_control_plan(whacker::app::CourtSide::Right);

    static_assert(whacker::app::human_axis_for_court_side(
        quick_plan,
        axes,
        whacker::app::CourtSide::Left) == -0.5f);
    static_assert(whacker::app::human_axis_for_court_side(
        quick_plan,
        axes,
        whacker::app::CourtSide::Right) == 0.75f);
    TEST_CHECK(whacker::app::human_axis_for_court_side(
        story_right_plan,
        axes,
        whacker::app::CourtSide::Right) == -0.5f);
    TEST_CHECK(whacker::app::human_axis_for_court_side(
        story_right_plan,
        axes,
        whacker::app::CourtSide::Left) == 0.75f);
}

void test_menu_intent_defaults_to_no_action() {
    constexpr whacker::app::MenuIntent intent {};

    static_assert(!intent.up);
    static_assert(!intent.down);
    static_assert(!intent.left);
    static_assert(!intent.right);
    static_assert(!intent.confirm);
    static_assert(!intent.back);
}

}  // namespace

int main() {
    test_quick_match_plan_keeps_desktop_slots_on_default_sides();
    test_story_player_plan_can_put_player_on_either_side();
    test_slot_and_side_lookup_round_trip_valid_plans();
    test_human_axis_lookup_uses_plan_slots();
    test_menu_intent_defaults_to_no_action();
    return 0;
}
