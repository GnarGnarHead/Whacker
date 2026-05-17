#pragma once

#include "story_state.hpp"

namespace whacker::app {

struct StoryRivalSpec {
    StoryRivalId id = StoryRivalId::None;
    const char* name = "RIVAL";
    AiStyle style = AiStyle::Balanced;
    whacker::progression::SkillState skills {};
};

constexpr float kStoryRivalSkillBudgetCap = whacker::progression::kSkillBudgetCap;

constexpr bool story_rival_skill_component_in_range(const float value) {
    return value >= 0.0f && value <= 1.0f;
}

constexpr bool story_rival_skill_triplet_valid(const whacker::progression::SkillState& skills) {
    const float sum = skills.edge + skills.power + skills.spin_inject;
    return
        story_rival_skill_component_in_range(skills.edge) &&
        story_rival_skill_component_in_range(skills.power) &&
        story_rival_skill_component_in_range(skills.spin_inject) &&
        sum <= kStoryRivalSkillBudgetCap + 1.0e-5f;
}

constexpr bool story_rival_spec_valid(const StoryRivalSpec& spec) {
    return story_rival_skill_triplet_valid(spec.skills);
}

inline constexpr StoryRivalSpec kStoryRivalNone {
    .id = StoryRivalId::None,
    .name = "RIVAL",
    .style = AiStyle::Balanced,
    .skills = {.edge = 0.0f, .power = 0.0f, .spin_inject = 0.0f},
};

// Rival identity/base profile table only. Story match routing and per-match tuning live in story_script_catalog.*.
inline constexpr StoryRivalSpec kStoryRivalKai {
    .id = StoryRivalId::Kai,
    .name = "KAI",
    .style = AiStyle::Balanced,
    .skills = {.edge = 0.12f, .power = 0.12f, .spin_inject = 0.12f},
};

inline constexpr StoryRivalSpec kStoryRivalAya {
    .id = StoryRivalId::Aya,
    .name = "AYA",
    .style = AiStyle::Balanced,
    .skills = {.edge = 0.17f, .power = 0.12f, .spin_inject = 0.12f},
};

inline constexpr StoryRivalSpec kStoryRivalBenji {
    .id = StoryRivalId::Benji,
    .name = "BENJI",
    .style = AiStyle::Spin,
    .skills = {.edge = 0.02f, .power = 0.04f, .spin_inject = 0.40f},
};

inline constexpr StoryRivalSpec kStoryRivalTix {
    .id = StoryRivalId::Tix,
    .name = "TIX",
    .style = AiStyle::Technical,
    .skills = {.edge = 0.30f, .power = 0.22f, .spin_inject = 0.32f},
};

inline constexpr StoryRivalSpec kStoryRivalIssa {
    .id = StoryRivalId::Issa,
    .name = "ISSA",
    .style = AiStyle::Spin,
    .skills = {.edge = 0.24f, .power = 0.20f, .spin_inject = 0.40f},
};

inline constexpr StoryRivalSpec kStoryRivalJolo {
    .id = StoryRivalId::Jolo,
    .name = "JOLO",
    .style = AiStyle::Power,
    .skills = {.edge = 0.18f, .power = 0.40f, .spin_inject = 0.22f},
};

inline constexpr StoryRivalSpec kStoryRivalJuno {
    .id = StoryRivalId::Juno,
    .name = "JUNO",
    .style = AiStyle::Spin,
    .skills = {.edge = 0.22f, .power = 0.18f, .spin_inject = 0.30f},
};

inline constexpr StoryRivalSpec kStoryRivalRook {
    .id = StoryRivalId::Rook,
    .name = "ROOK",
    .style = AiStyle::Power,
    .skills = {.edge = 0.22f, .power = 0.44f, .spin_inject = 0.20f},
};

inline constexpr StoryRivalSpec kStoryRivalMira {
    .id = StoryRivalId::Mira,
    .name = "MIRA",
    .style = AiStyle::Technical,
    .skills = {.edge = 0.42f, .power = 0.24f, .spin_inject = 0.26f},
};

inline constexpr StoryRivalSpec kStoryRivalVex {
    .id = StoryRivalId::Vex,
    .name = "VEX",
    .style = AiStyle::Spin,
    .skills = {.edge = 0.28f, .power = 0.24f, .spin_inject = 0.50f},
};

inline constexpr StoryRivalSpec kStoryRivalNova {
    .id = StoryRivalId::Nova,
    .name = "NOVA",
    .style = AiStyle::Balanced,
    .skills = {.edge = 0.56f, .power = 0.56f, .spin_inject = 0.56f},
};

static_assert(story_rival_spec_valid(kStoryRivalNone), "Story rival NONE spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalKai), "Story rival KAI spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalAya), "Story rival AYA spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalBenji), "Story rival BENJI spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalTix), "Story rival TIX spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalIssa), "Story rival ISSA spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalJolo), "Story rival JOLO spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalJuno), "Story rival JUNO spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalRook), "Story rival ROOK spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalMira), "Story rival MIRA spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalVex), "Story rival VEX spec violates skill constraints.");
static_assert(story_rival_spec_valid(kStoryRivalNova), "Story rival NOVA spec violates skill constraints.");

inline const StoryRivalSpec& story_rival_spec(const StoryRivalId id) {
    switch (id) {
        case StoryRivalId::Kai:
            return kStoryRivalKai;
        case StoryRivalId::Aya:
            return kStoryRivalAya;
        case StoryRivalId::Benji:
            return kStoryRivalBenji;
        case StoryRivalId::Tix:
            return kStoryRivalTix;
        case StoryRivalId::Issa:
            return kStoryRivalIssa;
        case StoryRivalId::Jolo:
            return kStoryRivalJolo;
        case StoryRivalId::Juno:
            return kStoryRivalJuno;
        case StoryRivalId::Rook:
            return kStoryRivalRook;
        case StoryRivalId::Mira:
            return kStoryRivalMira;
        case StoryRivalId::Vex:
            return kStoryRivalVex;
        case StoryRivalId::Nova:
            return kStoryRivalNova;
        case StoryRivalId::None:
        default:
            return kStoryRivalNone;
    }
}

}  // namespace whacker::app
