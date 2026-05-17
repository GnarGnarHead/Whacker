#include <cassert>
#include <string_view>

#include "story_portraits.hpp"

namespace {

void test_portrait_asset_filename_mapping() {
    using whacker::app::StoryPortraitId;

    assert(std::string_view(whacker::app::story_portrait_asset_filename(StoryPortraitId::Player)) ==
           "player_character_portrait.png");
    assert(std::string_view(whacker::app::story_portrait_asset_filename(StoryPortraitId::Kai)) ==
           "kai_alvarez_portrait.png");
    assert(std::string_view(whacker::app::story_portrait_asset_filename(StoryPortraitId::Aya)) ==
           "aya_villanueva_portrait.png");
    assert(std::string_view(whacker::app::story_portrait_asset_filename(StoryPortraitId::Benji)) ==
           "benji_santos_portrait.png");
    assert(std::string_view(whacker::app::story_portrait_asset_filename(StoryPortraitId::Tix)) ==
           "tino_tix_ramos_portrait.png");
    assert(std::string_view(whacker::app::story_portrait_asset_filename(StoryPortraitId::CoachReyes)) ==
           "coach_reyes_portrait.png");
    assert(std::string_view(whacker::app::story_portrait_asset_filename(StoryPortraitId::Issa)) ==
           "issa_flores_portrait.png");
    assert(std::string_view(whacker::app::story_portrait_asset_filename(StoryPortraitId::Jolo)) ==
           "jolo_marasigan_portrait.png");
    assert(std::string_view(
               whacker::app::story_portrait_asset_filename(StoryPortraitId::Champion1967PlayerStandin)) ==
           "champion_1967_player_standin_portrait.png");
    assert(std::string_view(
               whacker::app::story_portrait_asset_filename(StoryPortraitId::Champion1967Contender)) ==
           "champion_1967_contender_portrait.png");
    assert(whacker::app::story_portrait_asset_filename(StoryPortraitId::None) == nullptr);
}

void test_story_rival_portrait_mapping() {
    using whacker::app::StoryPortraitId;
    using whacker::app::StoryRivalId;

    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Kai) == StoryPortraitId::Kai);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Aya) == StoryPortraitId::Aya);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Benji) == StoryPortraitId::Benji);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Tix) == StoryPortraitId::Tix);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Issa) == StoryPortraitId::Issa);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Jolo) == StoryPortraitId::Jolo);

    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Juno) == StoryPortraitId::None);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Rook) == StoryPortraitId::None);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Mira) == StoryPortraitId::None);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Vex) == StoryPortraitId::None);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::Nova) == StoryPortraitId::None);
    assert(whacker::app::story_portrait_for_rival_id(StoryRivalId::None) == StoryPortraitId::None);
}

}  // namespace

int main() {
    test_portrait_asset_filename_mapping();
    test_story_rival_portrait_mapping();
    return 0;
}
