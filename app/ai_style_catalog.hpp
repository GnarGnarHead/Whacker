#pragma once

#include <array>
#include <cctype>
#include <cstdint>
#include <string_view>

#include "progression/skills.hpp"
#include "progression/style.hpp"

namespace whacker::app {

enum class AiStyle : std::uint8_t {
    Balanced = 0,
    Power = 1,
    Spin = 2,
    Technical = 3,
    Maxed = 4
};

struct AiStyleProfile {
    AiStyle id = AiStyle::Balanced;
    const char* name = "balanced";
    const char* display_name = "Balanced";
    whacker::progression::SkillState seed_skills {};
    whacker::progression::RivalStyleProfile profile {};
    float execution_gain = 0.56f;
};

constexpr int kAiStyleCount = 5;

inline const std::array<AiStyleProfile, kAiStyleCount>& ai_style_profiles() {
    static const std::array<AiStyleProfile, kAiStyleCount> kProfiles = {
        AiStyleProfile {
            .id = AiStyle::Balanced,
            .name = "balanced",
            .display_name = "Balanced",
            .seed_skills = {.edge = 0.28f, .power = 0.28f, .spin_inject = 0.28f},
            .profile = {.bias = {.edge = 0.28f, .power = 0.28f, .spin_inject = 0.28f},
                        .skill_floor = {},
                        .skill_ceiling = {.edge = 1.0f, .power = 1.0f, .spin_inject = 1.0f},
                        .bias_floor = {.edge = 0.2f, .power = 0.2f, .spin_inject = 0.2f},
                        .bias_ceiling = {.edge = 3.0f, .power = 3.0f, .spin_inject = 3.0f},
                        .adapt_gain = 0.15f},
            .execution_gain = 0.78f,
        },
        AiStyleProfile {
            .id = AiStyle::Power,
            .name = "power",
            .display_name = "Power",
            .seed_skills = {.edge = 0.1f, .power = 1.0f, .spin_inject = 0.1f},
            .profile = {.bias = {.edge = 0.1f, .power = 1.0f, .spin_inject = 0.1f},
                        .skill_floor = {},
                        .skill_ceiling = {.edge = 0.62f, .power = 1.0f, .spin_inject = 0.48f},
                        .bias_floor = {.edge = 0.2f, .power = 0.2f, .spin_inject = 0.2f},
                        .bias_ceiling = {.edge = 3.0f, .power = 3.0f, .spin_inject = 3.0f},
                        .adapt_gain = 0.16f},
            .execution_gain = 0.78f,
        },
        AiStyleProfile {
            .id = AiStyle::Spin,
            .name = "spin",
            .display_name = "Spin",
            .seed_skills = {.edge = 0.1f, .power = 0.1f, .spin_inject = 1.0f},
            .profile = {.bias = {.edge = 0.1f, .power = 0.1f, .spin_inject = 1.0f},
                        .skill_floor = {},
                        .skill_ceiling = {.edge = 0.74f, .power = 0.56f, .spin_inject = 1.0f},
                        .bias_floor = {.edge = 0.2f, .power = 0.2f, .spin_inject = 0.2f},
                        .bias_ceiling = {.edge = 3.0f, .power = 3.0f, .spin_inject = 3.0f},
                        .adapt_gain = 0.24f},
            .execution_gain = 0.78f,
        },
        AiStyleProfile {
            .id = AiStyle::Technical,
            .name = "technical",
            .display_name = "Technical",
            .seed_skills = {.edge = 1.0f, .power = 0.1f, .spin_inject = 0.1f},
            .profile = {.bias = {.edge = 1.0f, .power = 0.1f, .spin_inject = 0.1f},
                        .skill_floor = {},
                        .skill_ceiling = {.edge = 1.0f, .power = 0.92f, .spin_inject = 0.80f},
                        .bias_floor = {.edge = 0.2f, .power = 0.2f, .spin_inject = 0.2f},
                        .bias_ceiling = {.edge = 3.0f, .power = 3.0f, .spin_inject = 3.0f},
                        .adapt_gain = 0.18f},
            .execution_gain = 0.78f,
        },
        AiStyleProfile {
            .id = AiStyle::Maxed,
            .name = "maxed",
            .display_name = "Maxed",
            .seed_skills = {.edge = 1.0f, .power = 1.0f, .spin_inject = 1.0f},
            .profile = {.bias = {.edge = 1.0f, .power = 1.0f, .spin_inject = 1.0f},
                        .skill_floor = {},
                        .skill_ceiling = {.edge = 1.0f, .power = 1.0f, .spin_inject = 1.0f},
                        .bias_floor = {.edge = 0.2f, .power = 0.2f, .spin_inject = 0.2f},
                        .bias_ceiling = {.edge = 3.0f, .power = 3.0f, .spin_inject = 3.0f},
                        .adapt_gain = 0.12f},
            .execution_gain = 1.28f,
        },
    };
    return kProfiles;
}

inline const AiStyleProfile& ai_style_profile(const AiStyle style) {
    return ai_style_profiles()[static_cast<std::size_t>(style)];
}

inline const char* ai_style_name(const AiStyle style) {
    return ai_style_profile(style).name;
}

inline const char* ai_style_display_name(const AiStyle style) {
    return ai_style_profile(style).display_name;
}

inline bool parse_ai_style(const std::string_view text, AiStyle& out) {
    if (text.empty()) {
        return false;
    }
    if (text == "0") {
        out = AiStyle::Balanced;
        return true;
    }
    if (text == "1") {
        out = AiStyle::Power;
        return true;
    }
    if (text == "2") {
        out = AiStyle::Spin;
        return true;
    }
    if (text == "3") {
        out = AiStyle::Technical;
        return true;
    }
    if (text == "4") {
        out = AiStyle::Maxed;
        return true;
    }
    const auto equals_ignore_case = [](const std::string_view a, const std::string_view b) {
        if (a.size() != b.size()) {
            return false;
        }
        for (std::size_t i = 0; i < a.size(); ++i) {
            const unsigned char ca = static_cast<unsigned char>(a[i]);
            const unsigned char cb = static_cast<unsigned char>(b[i]);
            if (std::tolower(ca) != std::tolower(cb)) {
                return false;
            }
        }
        return true;
    };
    for (const AiStyleProfile& profile : ai_style_profiles()) {
        if (equals_ignore_case(text, profile.name) || equals_ignore_case(text, profile.display_name)) {
            out = profile.id;
            return true;
        }
    }
    return false;
}

inline int ai_style_index(const AiStyle style) {
    return static_cast<int>(style);
}

inline AiStyle ai_style_from_index(const int index) {
    switch (index) {
        case 0:
            return AiStyle::Balanced;
        case 1:
            return AiStyle::Power;
        case 2:
            return AiStyle::Spin;
        case 3:
            return AiStyle::Technical;
        case 4:
            return AiStyle::Maxed;
        default:
            return AiStyle::Balanced;
    }
}

}  // namespace whacker::app

