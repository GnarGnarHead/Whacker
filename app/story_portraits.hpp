#pragma once

#include <cstdint>

#include "story_state.hpp"

namespace whacker::app {

enum class StoryPortraitId : std::uint8_t {
    None = 0,
    Player = 1,
    Kai = 2,
    Aya = 3,
    Benji = 4,
    Tix = 5,
    CoachReyes = 6,
    Issa = 7,
    Jolo = 8,
    Champion1967PlayerStandin = 9,
    Champion1967Contender = 10,
};

const char* story_portrait_asset_filename(StoryPortraitId portrait_id);
StoryPortraitId story_portrait_for_rival_id(StoryRivalId rival_id);

}  // namespace whacker::app
