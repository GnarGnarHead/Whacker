#include "story_name_entry.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

#include "text_utils.hpp"

namespace whacker::app {

namespace {

constexpr std::string_view kDefaultStoryPlayerName = "PLAYER";
constexpr std::string_view kNameEntryAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -_";
constexpr std::size_t kMaxStoryNameLength = 16u;

char normalize_name_char(const char raw_ch) {
    if (raw_ch >= 'a' && raw_ch <= 'z') {
        return static_cast<char>(raw_ch - 'a' + 'A');
    }
    return raw_ch;
}

bool is_allowed_name_char(const char ch) {
    return (ch >= 'A' && ch <= 'Z') ||
        (ch >= '0' && ch <= '9') ||
        ch == ' ' ||
        ch == '-' ||
        ch == '_';
}

std::size_t clamp_selected_index(const StoryIntroState& story_intro_state) {
    if (story_intro_state.entered_name.empty()) {
        return 0u;
    }
    return std::min(
        story_intro_state.name_entry.selected_index,
        story_intro_state.entered_name.size() - 1u);
}

void normalize_selected_index(StoryIntroState& story_intro_state) {
    story_intro_state.name_entry.selected_index = clamp_selected_index(story_intro_state);
}

void clear_default_seed(StoryIntroState& story_intro_state) {
    story_intro_state.name_entry.default_seed_active = false;
}

void clear_feedback_after_edit(StoryIntroState& story_intro_state) {
    if (!story_intro_state.name_accept_pending && !story_intro_state.name_missing_prompt) {
        return;
    }
    story_intro_state.name_accept_pending = false;
    story_intro_state.name_missing_prompt = false;
    reset_story_intro_typewriter(story_intro_state);
}

void replace_seeded_default_before_text_input(StoryIntroState& story_intro_state) {
    if (!story_intro_state.name_entry.default_seed_active) {
        return;
    }
    story_intro_state.entered_name.clear();
    story_intro_state.name_entry.selected_index = 0u;
    clear_default_seed(story_intro_state);
}

bool append_text_input(StoryIntroState& story_intro_state, const std::string& text_input) {
    bool changed = false;
    for (const char raw_ch : text_input) {
        if (story_intro_state.entered_name.size() >= kMaxStoryNameLength) {
            return changed;
        }

        const char ch = normalize_name_char(raw_ch);
        if (!is_allowed_name_char(ch)) {
            continue;
        }
        if (ch == ' ' &&
            (story_intro_state.entered_name.empty() || story_intro_state.entered_name.back() == ' ')) {
            continue;
        }

        story_intro_state.entered_name.push_back(ch);
        story_intro_state.name_entry.selected_index = story_intro_state.entered_name.size() - 1u;
        changed = true;
    }
    if (changed) {
        clear_default_seed(story_intro_state);
    }
    return changed;
}

bool delete_selected_character(StoryIntroState& story_intro_state) {
    if (story_intro_state.entered_name.empty()) {
        clear_default_seed(story_intro_state);
        normalize_selected_index(story_intro_state);
        return false;
    }

    const std::size_t index = clamp_selected_index(story_intro_state);
    story_intro_state.entered_name.erase(index, 1u);
    if (story_intro_state.entered_name.empty()) {
        story_intro_state.name_entry.selected_index = 0u;
    } else if (index >= story_intro_state.entered_name.size()) {
        story_intro_state.name_entry.selected_index = story_intro_state.entered_name.size() - 1u;
    } else {
        story_intro_state.name_entry.selected_index = index;
    }
    clear_default_seed(story_intro_state);
    return true;
}

bool move_selected_character(StoryIntroState& story_intro_state, const int direction) {
    if (story_intro_state.entered_name.empty() || direction == 0) {
        return false;
    }

    const std::size_t previous = clamp_selected_index(story_intro_state);
    std::size_t next = previous;
    if (direction < 0 && previous > 0u) {
        next = previous - 1u;
    } else if (direction > 0 && previous + 1u < story_intro_state.entered_name.size()) {
        next = previous + 1u;
    } else if (direction > 0 && story_intro_state.entered_name.size() < kMaxStoryNameLength) {
        story_intro_state.entered_name.push_back('A');
        story_intro_state.name_entry.selected_index = story_intro_state.entered_name.size() - 1u;
        clear_default_seed(story_intro_state);
        return true;
    }
    story_intro_state.name_entry.selected_index = next;
    return next != previous;
}

bool cycle_selected_character(StoryIntroState& story_intro_state, const int direction) {
    if (direction == 0) {
        return false;
    }
    if (story_intro_state.entered_name.empty()) {
        story_intro_state.entered_name = "A";
        story_intro_state.name_entry.selected_index = 0u;
        clear_default_seed(story_intro_state);
        return true;
    }

    const std::size_t index = clamp_selected_index(story_intro_state);
    const char current = normalize_name_char(story_intro_state.entered_name[index]);
    const std::size_t current_pos = kNameEntryAlphabet.find(current);
    const std::size_t alphabet_size = kNameEntryAlphabet.size();
    const std::size_t base_pos = current_pos == std::string_view::npos ? 0u : current_pos;
    const std::size_t next_pos = direction > 0
        ? (base_pos + 1u) % alphabet_size
        : (base_pos + alphabet_size - 1u) % alphabet_size;
    const char next = kNameEntryAlphabet[next_pos];
    if (next == story_intro_state.entered_name[index]) {
        return false;
    }

    story_intro_state.entered_name[index] = next;
    story_intro_state.name_entry.selected_index = index;
    clear_default_seed(story_intro_state);
    return true;
}

std::string sanitize_story_name(
    const std::string& raw,
    const StoryIntroSanitizeNameFn sanitize_name_fn) {
    if (sanitize_name_fn != nullptr) {
        return sanitize_name_fn(raw);
    }
    const std::string trimmed = trim_copy(raw);
    return trimmed.empty() ? std::string {kDefaultStoryPlayerName} : trimmed;
}

}  // namespace

void reset_story_name_entry_editor(StoryIntroState& story_intro_state) {
    story_intro_state.name_entry = StoryNameEntryState {};
}

void prepare_story_name_entry(StoryIntroState& story_intro_state) {
    if (story_intro_state.name_entry.initialized) {
        normalize_selected_index(story_intro_state);
        return;
    }

    story_intro_state.name_entry.initialized = true;
    if (trim_copy(story_intro_state.entered_name).empty()) {
        story_intro_state.entered_name = std::string {kDefaultStoryPlayerName};
        story_intro_state.name_entry.default_seed_active = true;
    } else {
        story_intro_state.name_entry.default_seed_active = false;
    }
    normalize_selected_index(story_intro_state);
}

std::string story_name_entry_display_text(const StoryIntroState& story_intro_state) {
    const std::string_view name = story_intro_state.entered_name.empty() && !story_intro_state.name_entry.initialized
        ? kDefaultStoryPlayerName
        : std::string_view {story_intro_state.entered_name};
    if (name.empty()) {
        return "_";
    }

    const std::size_t selected = std::min(
        story_intro_state.name_entry.selected_index,
        name.size() - 1u);
    std::string display;
    display.reserve(name.size() + 2u);
    for (std::size_t i = 0; i < name.size(); ++i) {
        if (i == selected) {
            display.push_back('[');
        }
        display.push_back(name[i]);
        if (i == selected) {
            display.push_back(']');
        }
    }
    return display;
}

StoryNameEntryEditResult apply_story_name_entry_input(
    StoryIntroState& story_intro_state,
    const MenuIntent& intent,
    const StoryNameTextInput& text_input) {
    prepare_story_name_entry(story_intro_state);

    StoryNameEntryEditResult result {};
    const bool wants_back = intent.back || text_input.backspace_pressed;
    const bool wants_text = !text_input.text.empty();
    const bool wants_move = intent.left || intent.right || intent.up || intent.down;

    if (!wants_back && !wants_text && !wants_move) {
        return result;
    }

    if (story_intro_state.name_accept_pending) {
        story_intro_state.name_accept_pending = false;
        story_intro_state.name_missing_prompt = false;
        reset_story_intro_typewriter(story_intro_state);
        result.changed = true;
        if (wants_back && !wants_text && !wants_move) {
            result.consumed_back = true;
            return result;
        }
    }

    if (wants_text) {
        replace_seeded_default_before_text_input(story_intro_state);
        result.changed = append_text_input(story_intro_state, text_input.text) || result.changed;
    }

    if (wants_back) {
        result.consumed_back = true;
        result.changed = delete_selected_character(story_intro_state) || result.changed;
    }

    if (intent.left != intent.right) {
        result.changed = move_selected_character(story_intro_state, intent.right ? 1 : -1) || result.changed;
    }
    if (intent.up != intent.down) {
        result.changed = cycle_selected_character(story_intro_state, intent.up ? 1 : -1) || result.changed;
    }

    if (result.changed) {
        clear_feedback_after_edit(story_intro_state);
    }
    return result;
}

StoryNameEntryConfirmResult confirm_story_name_entry(
    StoryIntroState& story_intro_state,
    const StoryIntroSanitizeNameFn sanitize_name_fn) {
    prepare_story_name_entry(story_intro_state);

    if (!story_intro_state.name_accept_pending) {
        if (trim_copy(story_intro_state.entered_name).empty()) {
            story_intro_state.entered_name = std::string {kDefaultStoryPlayerName};
            story_intro_state.name_entry.selected_index = story_intro_state.entered_name.size() - 1u;
        }
        story_intro_state.name_accept_pending = true;
        story_intro_state.name_missing_prompt = false;
        reset_story_intro_typewriter(story_intro_state);
        return StoryNameEntryConfirmResult::ConfirmationStarted;
    }

    story_intro_state.entered_name = sanitize_story_name(story_intro_state.entered_name, sanitize_name_fn);
    story_intro_state.name_accept_pending = false;
    story_intro_state.name_missing_prompt = false;
    story_intro_state.phase = StoryIntroPhase::PlayMatch;
    story_intro_state.phase_timer = 0.0f;
    story_intro_state.dialogue_writing = false;
    reset_story_name_entry_editor(story_intro_state);
    return StoryNameEntryConfirmResult::Accepted;
}

}  // namespace whacker::app
