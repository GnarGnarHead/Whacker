#include <cassert>

#include "story_text.hpp"
#include "story_text_week.hpp"

namespace {

using whacker::app::StoryMatchKind;
using whacker::app::story_text::FeedbackLines;
using whacker::app::story_text_week::SceneKey;

void test_week_01_scene_catalog_has_required_entries() {
    constexpr std::string_view node = "club_week_01";

    assert(!whacker::app::story_text_week::scene_text(node, SceneKey::OnboardingEarlyArrivalHeader).empty());
    assert(!whacker::app::story_text_week::scene_text(node, SceneKey::OnboardingAyaEarlyArrivalLine2Template).empty());
    assert(!whacker::app::story_text_week::scene_text(node, SceneKey::AtHomeYoutubeLine1).empty());
    assert(!whacker::app::story_text_week::scene_text(node, SceneKey::AtHomeYoutubeLine2).empty());
    assert(!whacker::app::story_text_week::scene_text(node, SceneKey::TixMidweekSceneLine5).empty());
}

void test_unknown_node_returns_no_overrides() {
    constexpr std::string_view unknown_node = "club_week_99";
    assert(whacker::app::story_text_week::scene_text(
               unknown_node,
               SceneKey::OnboardingEarlyArrivalHeader)
               .empty());

    FeedbackLines feedback {};
    assert(!whacker::app::story_text_week::match_start_feedback(
        unknown_node,
        StoryMatchKind::Training,
        feedback));
}

void test_week_01_match_start_feedback_matches_current_defaults() {
    constexpr std::string_view node = "club_week_01";
    for (const StoryMatchKind kind : {
             StoryMatchKind::Training,
             StoryMatchKind::Official,
             StoryMatchKind::Imagination1967,
             StoryMatchKind::TixLunch,
         }) {
        FeedbackLines week_feedback {};
        const bool has_override =
            whacker::app::story_text_week::match_start_feedback(node, kind, week_feedback);
        assert(has_override);

        const FeedbackLines defaults = whacker::app::story_text::match_start_feedback(kind);
        assert(week_feedback.line_1 == defaults.line_1);
        assert(week_feedback.line_2 == defaults.line_2);
    }
}

void test_intro_forfeit_performance_line_uses_thank_you_text() {
    whacker::app::StoryIntroState intro {};
    intro.player_forfeited = true;
    const std::string forfeit_line = whacker::app::story_text::intro_performance_line(intro);
    assert(forfeit_line == "well thanks for the game.");
    assert(
        whacker::app::story_text::intro_rival_intro_line_2(4, 5, forfeit_line, true) ==
        "well thanks for the game.");
    assert(
        whacker::app::story_text::intro_rival_intro_line_2(4, 5, "That was close.", false) ==
        "FINAL 4-5  That was close.");
}

}  // namespace

int main() {
    test_week_01_scene_catalog_has_required_entries();
    test_unknown_node_returns_no_overrides();
    test_week_01_match_start_feedback_matches_current_defaults();
    test_intro_forfeit_performance_line_uses_thank_you_text();
    return 0;
}
