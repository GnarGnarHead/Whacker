#include "story_name_entry.hpp"
#include "test_assert.hpp"

#include <string>

namespace {

whacker::app::StoryIntroState make_name_entry_state() {
    whacker::app::StoryIntroState state {};
    state.phase = whacker::app::StoryIntroPhase::NameEntry;
    state.dialogue_writing = false;
    return state;
}

std::string sanitize_to_confirmed(const std::string& /*raw_name*/) {
    return "CONFIRMED";
}

void test_prepare_seeds_player_for_controller_editing() {
    whacker::app::StoryIntroState state = make_name_entry_state();

    whacker::app::prepare_story_name_entry(state);

    TEST_CHECK(state.entered_name == "PLAYER");
    TEST_CHECK(state.name_entry.initialized);
    TEST_CHECK(state.name_entry.default_seed_active);
    TEST_CHECK(whacker::app::story_name_entry_display_text(state) == "[P]LAYER");
}

void test_keyboard_text_replaces_untouched_default() {
    whacker::app::StoryIntroState state = make_name_entry_state();
    whacker::app::prepare_story_name_entry(state);

    const whacker::app::StoryNameEntryEditResult result =
        whacker::app::apply_story_name_entry_input(
            state,
            whacker::app::MenuIntent {},
            whacker::app::StoryNameTextInput {.text = "ace"});

    TEST_CHECK(result.changed);
    TEST_CHECK(!state.name_entry.default_seed_active);
    TEST_CHECK(state.entered_name == "ACE");
    TEST_CHECK(whacker::app::story_name_entry_display_text(state) == "AC[E]");
}

void test_controller_cycles_moves_and_deletes_name() {
    whacker::app::StoryIntroState state = make_name_entry_state();
    whacker::app::prepare_story_name_entry(state);

    whacker::app::MenuIntent intent {};
    intent.up = true;
    TEST_CHECK(whacker::app::apply_story_name_entry_input(state, intent, {}).changed);
    TEST_CHECK(state.entered_name == "QLAYER");
    TEST_CHECK(whacker::app::story_name_entry_display_text(state) == "[Q]LAYER");

    intent = whacker::app::MenuIntent {};
    intent.right = true;
    TEST_CHECK(whacker::app::apply_story_name_entry_input(state, intent, {}).changed);
    TEST_CHECK(whacker::app::story_name_entry_display_text(state) == "Q[L]AYER");

    intent = whacker::app::MenuIntent {};
    intent.back = true;
    const whacker::app::StoryNameEntryEditResult delete_result =
        whacker::app::apply_story_name_entry_input(state, intent, {});
    TEST_CHECK(delete_result.changed);
    TEST_CHECK(delete_result.consumed_back);
    TEST_CHECK(state.entered_name == "QAYER");
    TEST_CHECK(whacker::app::story_name_entry_display_text(state) == "Q[A]YER");
}

void test_controller_right_at_end_extends_name() {
    whacker::app::StoryIntroState state = make_name_entry_state();
    state.entered_name = "ACE";
    whacker::app::prepare_story_name_entry(state);

    whacker::app::MenuIntent intent {};
    intent.right = true;
    TEST_CHECK(whacker::app::apply_story_name_entry_input(state, intent, {}).changed);
    TEST_CHECK(whacker::app::apply_story_name_entry_input(state, intent, {}).changed);
    TEST_CHECK(whacker::app::apply_story_name_entry_input(state, intent, {}).changed);

    TEST_CHECK(state.entered_name == "ACEA");
    TEST_CHECK(whacker::app::story_name_entry_display_text(state) == "ACE[A]");
}

void test_empty_confirm_defaults_then_sanitizes_on_second_confirm() {
    whacker::app::StoryIntroState state = make_name_entry_state();
    state.entered_name = "   ";

    const whacker::app::StoryNameEntryConfirmResult first =
        whacker::app::confirm_story_name_entry(state, sanitize_to_confirmed);
    TEST_CHECK(first == whacker::app::StoryNameEntryConfirmResult::ConfirmationStarted);
    TEST_CHECK(state.phase == whacker::app::StoryIntroPhase::NameEntry);
    TEST_CHECK(state.name_accept_pending);
    TEST_CHECK(!state.name_missing_prompt);
    TEST_CHECK(state.entered_name == "PLAYER");

    state.dialogue_writing = false;
    const whacker::app::StoryNameEntryConfirmResult second =
        whacker::app::confirm_story_name_entry(state, sanitize_to_confirmed);
    TEST_CHECK(second == whacker::app::StoryNameEntryConfirmResult::Accepted);
    TEST_CHECK(state.phase == whacker::app::StoryIntroPhase::PlayMatch);
    TEST_CHECK(state.entered_name == "CONFIRMED");
    TEST_CHECK(!state.name_accept_pending);
}

void test_back_action_cancels_confirmation_without_leaving_story() {
    whacker::app::StoryIntroState state = make_name_entry_state();
    state.entered_name = "ACE";
    (void)whacker::app::confirm_story_name_entry(state, sanitize_to_confirmed);
    state.dialogue_writing = false;

    whacker::app::MenuIntent intent {};
    intent.back = true;
    const whacker::app::StoryNameEntryEditResult result =
        whacker::app::apply_story_name_entry_input(state, intent, {});

    TEST_CHECK(result.changed);
    TEST_CHECK(result.consumed_back);
    TEST_CHECK(state.phase == whacker::app::StoryIntroPhase::NameEntry);
    TEST_CHECK(!state.name_accept_pending);
    TEST_CHECK(state.entered_name == "ACE");
}

}  // namespace

int main() {
    test_prepare_seeds_player_for_controller_editing();
    test_keyboard_text_replaces_untouched_default();
    test_controller_cycles_moves_and_deletes_name();
    test_controller_right_at_end_extends_name();
    test_empty_confirm_defaults_then_sanitizes_on_second_confirm();
    test_back_action_cancels_confirmation_without_leaving_story();
    return 0;
}
