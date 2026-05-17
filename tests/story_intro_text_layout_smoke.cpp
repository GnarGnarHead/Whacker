#include <cassert>
#include <limits>
#include <string>

#include "story_intro.hpp"
#include "story_intro_text_layout.hpp"

namespace {

whacker::app::StoryIntroState make_intro_state(const whacker::app::StoryIntroPhase phase) {
    whacker::app::StoryIntroState state {};
    state.phase = phase;
    state.break_kind = whacker::app::StoryIntroBreak::None;
    state.visible_chars = std::numeric_limits<std::size_t>::max();
    state.dialogue_writing = false;
    state.entered_name = "PLAYER";
    state.rival_name = "KAI";
    state.final_left_score = 10;
    state.final_right_score = 12;
    state.player_won = false;
    state.player_forfeited = false;
    state.points_played = 6;
    return state;
}

void test_rival_intro_layout_overflows_in_compact_framebuffer() {
    whacker::app::StoryIntroState state = make_intro_state(whacker::app::StoryIntroPhase::RivalIntro);
    state.entered_name =
        "PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER";
    whacker::app::ControlBindings controls {};
    const whacker::app::StoryIntroBodyLayout layout =
        whacker::app::compute_story_intro_body_layout_for_framebuffer(
            320,
            180,
            state,
            controls,
            nullptr,
            nullptr);

    assert(!layout.rows.empty());
    assert(layout.latest_start_row > 0);
    assert(layout.max_scroll_rows > 0);
}

void test_between_balls_layout_without_overflow_has_zero_scroll() {
    whacker::app::StoryIntroState state = make_intro_state(whacker::app::StoryIntroPhase::BetweenBalls);
    state.break_kind = whacker::app::StoryIntroBreak::None;
    whacker::app::ControlBindings controls {};
    const whacker::app::StoryIntroBodyLayout layout =
        whacker::app::compute_story_intro_body_layout_for_framebuffer(
            960,
            540,
            state,
            controls,
            nullptr,
            nullptr);

    assert(layout.latest_start_row == 0);
    assert(layout.max_scroll_rows == 0);
}

void test_play_match_intro_layout_has_no_scroll_rows() {
    const whacker::app::StoryIntroState state = make_intro_state(whacker::app::StoryIntroPhase::PlayMatch);
    whacker::app::ControlBindings controls {};
    const whacker::app::StoryIntroBodyLayout layout =
        whacker::app::compute_story_intro_body_layout_for_framebuffer(
            960,
            540,
            state,
            controls,
            nullptr,
            nullptr);

    assert(layout.rows.empty());
    assert(layout.max_scroll_rows == 0);
    assert(whacker::app::first_visible_story_intro_row_index(layout, 0) == 0);
}

void test_scroll_helpers_clamp_and_resolve_first_visible_row() {
    whacker::app::StoryIntroBodyLayout layout {};
    layout.latest_start_row = 9;
    layout.max_scroll_rows = 9;

    assert(whacker::app::clamp_story_intro_scroll_from_bottom(layout, -3) == 0);
    assert(whacker::app::clamp_story_intro_scroll_from_bottom(layout, 4) == 4);
    assert(whacker::app::clamp_story_intro_scroll_from_bottom(layout, 99) == 9);
    assert(whacker::app::first_visible_story_intro_row_index(layout, 0) == 9);
    assert(whacker::app::first_visible_story_intro_row_index(layout, 3) == 6);
    assert(whacker::app::first_visible_story_intro_row_index(layout, 9) == 0);
}

}  // namespace

int main() {
    test_rival_intro_layout_overflows_in_compact_framebuffer();
    test_between_balls_layout_without_overflow_has_zero_scroll();
    test_play_match_intro_layout_has_no_scroll_rows();
    test_scroll_helpers_clamp_and_resolve_first_visible_row();
    return 0;
}
