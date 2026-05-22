#include <cassert>
#include <string>

#include "overlay_layout_math.hpp"
#include "text_wrap.hpp"

namespace {

bool ends_with(const std::string& value, const std::string& suffix) {
    if (value.size() < suffix.size()) {
        return false;
    }
    return value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void test_choose_best_fitting_variant_uses_first_that_fits() {
    const std::string text = whacker::app::choose_best_fitting_variant(
        {"UP/DOWN MOVE  ENTER SELECT  ESC BACK", "ARROWS  ENTER  ESC"},
        40);
    assert(text == "UP/DOWN MOVE  ENTER SELECT  ESC BACK");
}

void test_choose_best_fitting_variant_falls_back_to_compact_variant() {
    const std::string text = whacker::app::choose_best_fitting_variant(
        {"UP/DOWN MOVE  LEFT/RIGHT ADJUST  ENTER SELECT  ESC BACK", "ARROWS MOVE  L/R ADJUST  ENTER  ESC"},
        35);
    assert(text == "ARROWS MOVE  L/R ADJUST  ENTER  ESC");
}

void test_choose_best_fitting_variant_ellipsizes_when_nothing_fits() {
    const std::string text = whacker::app::choose_best_fitting_variant(
        {"UP/DOWN MOVE  LEFT/RIGHT ADJUST  ENTER SELECT  ESC BACK", "ARROWS MOVE  L/R ADJUST  ENTER  ESC"},
        12);
    assert(static_cast<int>(text.size()) <= 12);
    assert(ends_with(text, "..."));
}

void test_fit_text_to_single_line_ellipsis_limits_output_width() {
    const std::string text =
        whacker::app::fit_text_to_single_line_ellipsis("THIS IS A VERY LONG LINE FOR A SMALL PANEL", 10);
    assert(static_cast<int>(text.size()) <= 10);
    assert(ends_with(text, "..."));
}

void test_chat_style_budget_prefers_compact_variant() {
    const int max_chars = whacker::app::max_chars_for_safe_text_width(360.0f, 1.9f, 0, 2, 8.0f);
    const std::string text = whacker::app::choose_best_fitting_variant(
        {"ENTER/SPACE SKIP  HOLD FAST", "ENTER SKIP  HOLD FAST"},
        max_chars);
    assert(text == "ENTER SKIP  HOLD FAST");
}

void test_chat_style_budget_ellipsizes_compact_variant_when_readable_text_is_tight() {
    const int max_chars = whacker::app::max_chars_for_safe_text_width(214.0f, 1.9f, 0, 2, 8.0f);
    const std::string text = whacker::app::choose_best_fitting_variant(
        {"ENTER/SPACE SKIP  HOLD FAST", "ENTER SKIP  HOLD FAST"},
        max_chars);
    assert(static_cast<int>(text.size()) <= max_chars);
    assert(ends_with(text, "..."));
}

void test_chat_style_budget_ellipsis_still_respects_limit() {
    const int max_chars = whacker::app::max_chars_for_safe_text_width(180.0f, 2.2f, 0, 2, 8.0f);
    const std::string text = whacker::app::fit_text_to_single_line_ellipsis(
        "THIS LINE SHOULD NEVER BLEED PAST THE CHAT EDGE",
        max_chars);
    assert(static_cast<int>(text.size()) <= max_chars);
}

}  // namespace

int main() {
    test_choose_best_fitting_variant_uses_first_that_fits();
    test_choose_best_fitting_variant_falls_back_to_compact_variant();
    test_choose_best_fitting_variant_ellipsizes_when_nothing_fits();
    test_fit_text_to_single_line_ellipsis_limits_output_width();
    test_chat_style_budget_prefers_compact_variant();
    test_chat_style_budget_ellipsizes_compact_variant_when_readable_text_is_tight();
    test_chat_style_budget_ellipsis_still_respects_limit();
    return 0;
}
