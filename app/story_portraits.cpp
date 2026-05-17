#include "story_portraits.hpp"

namespace whacker::app {

const char* story_portrait_asset_filename(const StoryPortraitId portrait_id) {
    switch (portrait_id) {
        case StoryPortraitId::Player:
            return "player_character_portrait.png";
        case StoryPortraitId::Kai:
            return "kai_alvarez_portrait.png";
        case StoryPortraitId::Aya:
            return "aya_villanueva_portrait.png";
        case StoryPortraitId::Benji:
            return "benji_santos_portrait.png";
        case StoryPortraitId::Tix:
            return "tino_tix_ramos_portrait.png";
        case StoryPortraitId::CoachReyes:
            return "coach_reyes_portrait.png";
        case StoryPortraitId::Issa:
            return "issa_flores_portrait.png";
        case StoryPortraitId::Jolo:
            return "jolo_marasigan_portrait.png";
        case StoryPortraitId::Champion1967PlayerStandin:
            return "champion_1967_player_standin_portrait.png";
        case StoryPortraitId::Champion1967Contender:
            return "champion_1967_contender_portrait.png";
        case StoryPortraitId::None:
        default:
            return nullptr;
    }
}

StoryPortraitId story_portrait_for_rival_id(const StoryRivalId rival_id) {
    switch (rival_id) {
        case StoryRivalId::Kai:
            return StoryPortraitId::Kai;
        case StoryRivalId::Aya:
            return StoryPortraitId::Aya;
        case StoryRivalId::Benji:
            return StoryPortraitId::Benji;
        case StoryRivalId::Tix:
            return StoryPortraitId::Tix;
        case StoryRivalId::Issa:
            return StoryPortraitId::Issa;
        case StoryRivalId::Jolo:
            return StoryPortraitId::Jolo;
        case StoryRivalId::None:
        case StoryRivalId::Juno:
        case StoryRivalId::Rook:
        case StoryRivalId::Mira:
        case StoryRivalId::Vex:
        case StoryRivalId::Nova:
        default:
            return StoryPortraitId::None;
    }
}

}  // namespace whacker::app
