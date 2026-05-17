#include <array>
#include <cctype>
#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "story_state.hpp"
#include "story_text_week.hpp"
#include "menu_sticker_pack.hpp"

namespace {

namespace fs = std::filesystem;

using whacker::app::StoryMatchKind;
using whacker::app::StoryRivalId;
using whacker::app::MenuStickerAnchor;
using whacker::app::MenuStickerPlacement;
using whacker::app::MenuStickerSizeMode;
using whacker::app::MenuStickerSurface;
using whacker::app::story_text_week::SceneKey;

struct ParseError {
    fs::path path;
    int line = 0;
    std::string message;
};

struct SceneTextEntry {
    std::string key;
    std::string text;
};

struct MatchStartFeedbackEntry {
    std::string match_kind;
    std::string line_1;
    std::string line_2;
};

struct HubNodeEntry {
    std::string node_id;
    int display_week = 0;
    StoryRivalId training_rival_id = StoryRivalId::None;
    StoryRivalId official_rival_id = StoryRivalId::None;
    std::string next_node_id;
};

struct Week01TextPack {
    fs::path source_path;
    std::string type;
    std::string pack_id;
    std::string node_id;
    std::vector<SceneTextEntry> scene_text;
    std::vector<MatchStartFeedbackEntry> match_start_feedback;
};

struct Season1GraphPack {
    fs::path source_path;
    std::string type;
    std::string pack_id;
    std::string season_id;
    std::vector<HubNodeEntry> hub_nodes;
};

struct MenuStickerSlotEntry {
    std::string asset_filename;
    MenuStickerAnchor anchor = MenuStickerAnchor::TopLeft;
    MenuStickerPlacement placement = MenuStickerPlacement::Edge;
    float x_norm = 0.5f;
    float y_norm = 0.5f;
    MenuStickerSizeMode size_mode = MenuStickerSizeMode::Scale;
    float scale = 0.20f;
    float height_px = 96.0f;
    float offset_x_px = 0.0f;
    float offset_y_px = 0.0f;
    float rotation_deg = 0.0f;
    bool allow_protected_overlap = false;
    int z_order = 0;
    bool has_asset = false;
    bool has_anchor = false;
    bool has_x_norm = false;
    bool has_y_norm = false;
    bool has_scale = false;
    bool has_height_px = false;
};

struct MenuStickerSurfaceEntry {
    MenuStickerSurface surface = MenuStickerSurface::MainMenu;
    std::vector<MenuStickerSlotEntry> slots;
    bool has_surface = false;
};

struct MenuStickerPack {
    fs::path source_path;
    std::string type;
    std::string pack_id;
    std::vector<MenuStickerSurfaceEntry> surfaces;
};

constexpr std::string_view kExpectedType = "week_text";
constexpr std::string_view kWeek01NodeId = "club_week_01";
constexpr std::string_view kExpectedSeasonGraphType = "season_graph";
constexpr std::string_view kExpectedSeason1Id = "season1";
constexpr std::string_view kExpectedMenuStickersType = "menu_stickers";

struct SceneKeyMapping {
    std::string_view name;
    SceneKey key;
};

constexpr std::array<SceneKeyMapping, 34> kWeek01SceneKeys = {{
    {"OnboardingEarlyArrivalHeader", SceneKey::OnboardingEarlyArrivalHeader},
    {"OnboardingClubFloorHeader", SceneKey::OnboardingClubFloorHeader},
    {"OnboardingCoachReyesHeader", SceneKey::OnboardingCoachReyesHeader},
    {"AtHomeYoutubeHeader", SceneKey::AtHomeYoutubeHeader},
    {"TixMidweekSceneHeader", SceneKey::TixMidweekSceneHeader},
    {"OnboardingAyaEarlyArrivalLine1", SceneKey::OnboardingAyaEarlyArrivalLine1},
    {"OnboardingAyaEarlyArrivalLine2Template", SceneKey::OnboardingAyaEarlyArrivalLine2Template},
    {"OnboardingAyaEarlyArrivalLine3", SceneKey::OnboardingAyaEarlyArrivalLine3},
    {"OnboardingAyaEarlyArrivalLine4", SceneKey::OnboardingAyaEarlyArrivalLine4},
    {"OnboardingAyaIntroToCoachTemplate", SceneKey::OnboardingAyaIntroToCoachTemplate},
    {"OnboardingCoachIntroPlayerTemplate", SceneKey::OnboardingCoachIntroPlayerTemplate},
    {"OnboardingCoachWelcomeLine", SceneKey::OnboardingCoachWelcomeLine},
    {"OnboardingCoachAssignBenjiLine", SceneKey::OnboardingCoachAssignBenjiLine},
    {"OnboardingCoachBenjiSpinWarningLine", SceneKey::OnboardingCoachBenjiSpinWarningLine},
    {"OnboardingCoachEntryRetryLine", SceneKey::OnboardingCoachEntryRetryLine},
    {"OnboardingCoachDayEndLine", SceneKey::OnboardingCoachDayEndLine},
    {"OnboardingCoachTrainingOpenLine", SceneKey::OnboardingCoachTrainingOpenLine},
    {"OnboardingCoachTrainingRepsLine", SceneKey::OnboardingCoachTrainingRepsLine},
    {"OnboardingTixPostDayLine1", SceneKey::OnboardingTixPostDayLine1},
    {"OnboardingTixPostDayLine2", SceneKey::OnboardingTixPostDayLine2},
    {"OnboardingTixPostDayLine3", SceneKey::OnboardingTixPostDayLine3},
    {"OnboardingTixPostDayLine4", SceneKey::OnboardingTixPostDayLine4},
    {"OnboardingTixPostDayLine5", SceneKey::OnboardingTixPostDayLine5},
    {"AtHomeYoutubeLine1", SceneKey::AtHomeYoutubeLine1},
    {"AtHomeYoutubeLine2", SceneKey::AtHomeYoutubeLine2},
    {"ImaginationTakeoverCueLine", SceneKey::ImaginationTakeoverCueLine},
    {"TixMidweekSceneLine1", SceneKey::TixMidweekSceneLine1},
    {"TixMidweekSceneLine2", SceneKey::TixMidweekSceneLine2},
    {"TixMidweekSceneLine3", SceneKey::TixMidweekSceneLine3},
    {"TixMidweekSceneLine4", SceneKey::TixMidweekSceneLine4},
    {"TixMidweekSceneLine5", SceneKey::TixMidweekSceneLine5},
    {"OnboardingAyaForfeitFeedbackLine", SceneKey::OnboardingAyaForfeitFeedbackLine},
    {"TixPostLunchLine1", SceneKey::TixPostLunchLine1},
    {"TixPostLunchLine2", SceneKey::TixPostLunchLine2},
}};

struct MatchKindMapping {
    std::string_view name;
    StoryMatchKind kind;
};

constexpr std::array<MatchKindMapping, 4> kWeek01FeedbackKinds = {{
    {"Training", StoryMatchKind::Training},
    {"Official", StoryMatchKind::Official},
    {"Imagination1967", StoryMatchKind::Imagination1967},
    {"TixLunch", StoryMatchKind::TixLunch},
}};

std::string_view trim(std::string_view input) {
    while (!input.empty() && std::isspace(static_cast<unsigned char>(input.front())) != 0) {
        input.remove_prefix(1);
    }
    while (!input.empty() && std::isspace(static_cast<unsigned char>(input.back())) != 0) {
        input.remove_suffix(1);
    }
    return input;
}

std::string strip_toml_comment(const std::string_view input) {
    bool in_string = false;
    bool escaping = false;
    for (std::size_t i = 0; i < input.size(); ++i) {
        const char c = input[i];
        if (escaping) {
            escaping = false;
            continue;
        }
        if (c == '\\' && in_string) {
            escaping = true;
            continue;
        }
        if (c == '"') {
            in_string = !in_string;
            continue;
        }
        if (!in_string && c == '#') {
            return std::string {input.substr(0, i)};
        }
    }
    return std::string {input};
}

std::optional<std::string> parse_toml_string(
    const fs::path& path,
    const int line,
    const std::string_view value,
    ParseError& out_error) {
    const std::string_view trimmed = trim(value);
    if (trimmed.size() < 2 || trimmed.front() != '"' || trimmed.back() != '"') {
        out_error = {
            .path = path,
            .line = line,
            .message = "Expected a double-quoted string.",
        };
        return std::nullopt;
    }
    std::string out;
    out.reserve(trimmed.size());
    bool escaping = false;
    for (std::size_t i = 1; i + 1 < trimmed.size(); ++i) {
        const char c = trimmed[i];
        if (escaping) {
            escaping = false;
            switch (c) {
                case '\\':
                    out.push_back('\\');
                    break;
                case '"':
                    out.push_back('"');
                    break;
                case 'n':
                    out.push_back('\n');
                    break;
                case 'r':
                    out.push_back('\r');
                    break;
                case 't':
                    out.push_back('\t');
                    break;
                default:
                    out_error = {
                        .path = path,
                        .line = line,
                        .message = std::string {"Unsupported escape sequence: \\"} + c,
                    };
                    return std::nullopt;
            }
            continue;
        }
        if (c == '\\') {
            escaping = true;
            continue;
        }
        out.push_back(c);
    }
    if (escaping) {
        out_error = {
            .path = path,
            .line = line,
            .message = "Unterminated escape sequence at end of string.",
        };
        return std::nullopt;
    }
    return out;
}

std::optional<int> parse_toml_int(
    const fs::path& path,
    const int line,
    const std::string_view value,
    ParseError& out_error) {
    const std::string_view trimmed = trim(value);
    if (trimmed.empty()) {
        out_error = {
            .path = path,
            .line = line,
            .message = "Expected an integer value.",
        };
        return std::nullopt;
    }

    int parsed = 0;
    const char* begin = trimmed.data();
    const char* end = trimmed.data() + trimmed.size();
    const std::from_chars_result result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc() || result.ptr != end) {
        out_error = {
            .path = path,
            .line = line,
            .message = "Expected a base-10 integer literal.",
        };
        return std::nullopt;
    }
    return parsed;
}

std::optional<float> parse_toml_float(
    const fs::path& path,
    const int line,
    const std::string_view value,
    ParseError& out_error) {
    const std::string_view trimmed = trim(value);
    if (trimmed.empty()) {
        out_error = {
            .path = path,
            .line = line,
            .message = "Expected a float value.",
        };
        return std::nullopt;
    }

    std::string parse_buffer(trimmed);
    char* end_ptr = nullptr;
    const float parsed = std::strtof(parse_buffer.c_str(), &end_ptr);
    if (end_ptr == parse_buffer.c_str() || (end_ptr != nullptr && *end_ptr != '\0')) {
        out_error = {
            .path = path,
            .line = line,
            .message = "Expected a decimal float literal.",
        };
        return std::nullopt;
    }
    return parsed;
}

std::optional<bool> parse_toml_bool(
    const fs::path& path,
    const int line,
    const std::string_view value,
    ParseError& out_error) {
    const std::string_view trimmed = trim(value);
    if (trimmed == "true") {
        return true;
    }
    if (trimmed == "false") {
        return false;
    }
    out_error = {
        .path = path,
        .line = line,
        .message = "Expected boolean literal true/false.",
    };
    return std::nullopt;
}

std::optional<SceneKey> parse_scene_key(std::string_view name) {
    for (const SceneKeyMapping& mapping : kWeek01SceneKeys) {
        if (mapping.name == name) {
            return mapping.key;
        }
    }
    return std::nullopt;
}

std::optional<StoryMatchKind> parse_week01_feedback_match_kind(std::string_view name) {
    for (const MatchKindMapping& mapping : kWeek01FeedbackKinds) {
        if (mapping.name == name) {
            return mapping.kind;
        }
    }
    return std::nullopt;
}

std::string escape_cpp_string_literal(std::string_view text) {
    std::string out;
    out.reserve(text.size() + 8);
    for (const char c : text) {
        switch (c) {
            case '\\':
                out += "\\\\";
                break;
            case '"':
                out += "\\\"";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                out.push_back(c);
                break;
        }
    }
    return out;
}

std::string format_cpp_float_literal(const float value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(4) << value;
    std::string rendered = out.str();
    if (const std::size_t dot = rendered.find('.'); dot != std::string::npos) {
        while (!rendered.empty() && rendered.back() == '0') {
            rendered.pop_back();
        }
        if (!rendered.empty() && rendered.back() == '.') {
            rendered.push_back('0');
        }
    }
    rendered.push_back('f');
    return rendered;
}

std::string format_error(const ParseError& error) {
    std::ostringstream message;
    message << error.path.string() << ":" << error.line << ": " << error.message;
    return message.str();
}

std::optional<std::string> parse_pack_type(const fs::path& path, ParseError& out_error) {
    std::ifstream input(path);
    if (!input.is_open()) {
        out_error = {.path = path, .line = 0, .message = "Failed to open file."};
        return std::nullopt;
    }

    std::string raw;
    int line = 0;
    while (std::getline(input, raw)) {
        ++line;
        const std::string uncommented = strip_toml_comment(raw);
        const std::string_view trimmed = trim(uncommented);
        if (trimmed.empty()) {
            continue;
        }
        if (trimmed.size() >= 4 && trimmed.starts_with("[[") && trimmed.ends_with("]]")) {
            out_error = {.path = path, .line = line, .message = "Missing required top-level key: type"};
            return std::nullopt;
        }

        const std::size_t eq = trimmed.find('=');
        if (eq == std::string_view::npos) {
            out_error = {.path = path, .line = line, .message = "Expected key = value assignment."};
            return std::nullopt;
        }
        const std::string_view key = trim(trimmed.substr(0, eq));
        const std::string_view value = trim(trimmed.substr(eq + 1));
        if (key == "type") {
            return parse_toml_string(path, line, value, out_error);
        }
    }

    out_error = {.path = path, .line = line, .message = "Missing required top-level key: type"};
    return std::nullopt;
}

std::optional<StoryRivalId> parse_story_rival_id(std::string_view name) {
    if (name == "None") {
        return StoryRivalId::None;
    }
    if (name == "Kai") {
        return StoryRivalId::Kai;
    }
    if (name == "Aya") {
        return StoryRivalId::Aya;
    }
    if (name == "Benji") {
        return StoryRivalId::Benji;
    }
    if (name == "Tix") {
        return StoryRivalId::Tix;
    }
    if (name == "Issa") {
        return StoryRivalId::Issa;
    }
    if (name == "Jolo") {
        return StoryRivalId::Jolo;
    }
    if (name == "Juno") {
        return StoryRivalId::Juno;
    }
    if (name == "Rook") {
        return StoryRivalId::Rook;
    }
    if (name == "Mira") {
        return StoryRivalId::Mira;
    }
    if (name == "Vex") {
        return StoryRivalId::Vex;
    }
    if (name == "Nova") {
        return StoryRivalId::Nova;
    }
    return std::nullopt;
}

std::string_view story_rival_id_cpp_name(const StoryRivalId id) {
    switch (id) {
        case StoryRivalId::Kai:
            return "Kai";
        case StoryRivalId::Aya:
            return "Aya";
        case StoryRivalId::Benji:
            return "Benji";
        case StoryRivalId::Tix:
            return "Tix";
        case StoryRivalId::Issa:
            return "Issa";
        case StoryRivalId::Jolo:
            return "Jolo";
        case StoryRivalId::Juno:
            return "Juno";
        case StoryRivalId::Rook:
            return "Rook";
        case StoryRivalId::Mira:
            return "Mira";
        case StoryRivalId::Vex:
            return "Vex";
        case StoryRivalId::Nova:
            return "Nova";
        case StoryRivalId::None:
        default:
            return "None";
    }
}

std::optional<MenuStickerSurface> parse_menu_sticker_surface(std::string_view name) {
    if (name == "main_menu") {
        return MenuStickerSurface::MainMenu;
    }
    if (name == "main_menu_back") {
        return MenuStickerSurface::MainMenuBack;
    }
    if (name == "story_menu") {
        return MenuStickerSurface::StoryMenu;
    }
    if (name == "story_hub") {
        return MenuStickerSurface::StoryHub;
    }
    if (name == "quick_setup") {
        return MenuStickerSurface::QuickSetup;
    }
    if (name == "options_menu") {
        return MenuStickerSurface::OptionsMenu;
    }
    if (name == "pause_menu") {
        return MenuStickerSurface::PauseMenu;
    }
    return std::nullopt;
}

std::string_view menu_sticker_surface_cpp_name(const MenuStickerSurface surface) {
    switch (surface) {
        case MenuStickerSurface::MainMenuBack:
            return "MainMenuBack";
        case MenuStickerSurface::StoryMenu:
            return "StoryMenu";
        case MenuStickerSurface::StoryHub:
            return "StoryHub";
        case MenuStickerSurface::QuickSetup:
            return "QuickSetup";
        case MenuStickerSurface::OptionsMenu:
            return "OptionsMenu";
        case MenuStickerSurface::PauseMenu:
            return "PauseMenu";
        case MenuStickerSurface::MainMenu:
        default:
            return "MainMenu";
    }
}

std::optional<MenuStickerAnchor> parse_menu_sticker_anchor(std::string_view name) {
    if (name == "top_left") {
        return MenuStickerAnchor::TopLeft;
    }
    if (name == "top_right") {
        return MenuStickerAnchor::TopRight;
    }
    if (name == "bottom_left") {
        return MenuStickerAnchor::BottomLeft;
    }
    if (name == "bottom_right") {
        return MenuStickerAnchor::BottomRight;
    }
    if (name == "left_edge") {
        return MenuStickerAnchor::LeftEdge;
    }
    if (name == "right_edge") {
        return MenuStickerAnchor::RightEdge;
    }
    return std::nullopt;
}

std::string_view menu_sticker_anchor_cpp_name(const MenuStickerAnchor anchor) {
    switch (anchor) {
        case MenuStickerAnchor::TopRight:
            return "TopRight";
        case MenuStickerAnchor::BottomLeft:
            return "BottomLeft";
        case MenuStickerAnchor::BottomRight:
            return "BottomRight";
        case MenuStickerAnchor::LeftEdge:
            return "LeftEdge";
        case MenuStickerAnchor::RightEdge:
            return "RightEdge";
        case MenuStickerAnchor::TopLeft:
        default:
            return "TopLeft";
    }
}

std::optional<MenuStickerPlacement> parse_menu_sticker_placement(std::string_view name) {
    if (name == "edge") {
        return MenuStickerPlacement::Edge;
    }
    if (name == "inside") {
        return MenuStickerPlacement::Inside;
    }
    return std::nullopt;
}

std::string_view menu_sticker_placement_cpp_name(const MenuStickerPlacement placement) {
    switch (placement) {
        case MenuStickerPlacement::Inside:
            return "Inside";
        case MenuStickerPlacement::Edge:
        default:
            return "Edge";
    }
}

std::optional<MenuStickerSizeMode> parse_menu_sticker_size_mode(std::string_view name) {
    if (name == "scale") {
        return MenuStickerSizeMode::Scale;
    }
    if (name == "height_px") {
        return MenuStickerSizeMode::HeightPx;
    }
    return std::nullopt;
}

std::string_view menu_sticker_size_mode_cpp_name(const MenuStickerSizeMode size_mode) {
    switch (size_mode) {
        case MenuStickerSizeMode::HeightPx:
            return "HeightPx";
        case MenuStickerSizeMode::Scale:
        default:
            return "Scale";
    }
}

std::optional<Week01TextPack> parse_week01_text_pack(const fs::path& path, ParseError& out_error) {
    std::ifstream input(path);
    if (!input.is_open()) {
        out_error = {.path = path, .line = 0, .message = "Failed to open file."};
        return std::nullopt;
    }

    Week01TextPack pack {};
    pack.source_path = path;

    enum class ActiveTable : std::uint8_t {
        None = 0,
        SceneText = 1,
        MatchStartFeedback = 2,
    };
    ActiveTable table = ActiveTable::None;
    SceneTextEntry* active_scene_text = nullptr;
    MatchStartFeedbackEntry* active_feedback = nullptr;

    std::string raw;
    int line = 0;
    while (std::getline(input, raw)) {
        ++line;
        const std::string uncommented = strip_toml_comment(raw);
        const std::string_view trimmed = trim(uncommented);
        if (trimmed.empty()) {
            continue;
        }

        if (trimmed.size() >= 4 && trimmed.starts_with("[[") && trimmed.ends_with("]]")) {
            const std::string_view table_name = trim(trimmed.substr(2, trimmed.size() - 4));
            active_scene_text = nullptr;
            active_feedback = nullptr;
            if (table_name == "scene_text") {
                pack.scene_text.push_back(SceneTextEntry {});
                active_scene_text = &pack.scene_text.back();
                table = ActiveTable::SceneText;
                continue;
            }
            if (table_name == "match_start_feedback") {
                pack.match_start_feedback.push_back(MatchStartFeedbackEntry {});
                active_feedback = &pack.match_start_feedback.back();
                table = ActiveTable::MatchStartFeedback;
                continue;
            }
            out_error = {.path = path, .line = line, .message = std::string {"Unknown table: "} + std::string {table_name}};
            return std::nullopt;
        }

        const std::size_t eq = trimmed.find('=');
        if (eq == std::string_view::npos) {
            out_error = {.path = path, .line = line, .message = "Expected key = value assignment."};
            return std::nullopt;
        }
        const std::string_view key = trim(trimmed.substr(0, eq));
        const std::string_view value = trim(trimmed.substr(eq + 1));
        if (key.empty()) {
            out_error = {.path = path, .line = line, .message = "Empty key in assignment."};
            return std::nullopt;
        }

        if (table == ActiveTable::None) {
            if (key == "type") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                pack.type = *parsed;
            } else if (key == "pack_id") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                pack.pack_id = *parsed;
            } else if (key == "node_id") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                pack.node_id = *parsed;
            } else {
                out_error = {.path = path, .line = line, .message = std::string {"Unknown top-level key: "} + std::string {key}};
                return std::nullopt;
            }
            continue;
        }

        if (table == ActiveTable::SceneText) {
            if (active_scene_text == nullptr) {
                out_error = {.path = path, .line = line, .message = "Internal parser error (no active scene_text)."};
                return std::nullopt;
            }
            if (key == "key") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_scene_text->key = *parsed;
            } else if (key == "text") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_scene_text->text = *parsed;
            } else {
                out_error = {.path = path, .line = line, .message = std::string {"Unknown scene_text key: "} + std::string {key}};
                return std::nullopt;
            }
            continue;
        }

        if (table == ActiveTable::MatchStartFeedback) {
            if (active_feedback == nullptr) {
                out_error = {.path = path, .line = line, .message = "Internal parser error (no active match_start_feedback)."};
                return std::nullopt;
            }
            if (key == "match_kind") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_feedback->match_kind = *parsed;
            } else if (key == "line_1") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_feedback->line_1 = *parsed;
            } else if (key == "line_2") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_feedback->line_2 = *parsed;
            } else {
                out_error = {.path = path, .line = line, .message = std::string {"Unknown match_start_feedback key: "} + std::string {key}};
                return std::nullopt;
            }
            continue;
        }
    }

    if (pack.type != kExpectedType) {
        out_error = {.path = path, .line = 0, .message = "Pack type must be \"week_text\"."};
        return std::nullopt;
    }
    if (pack.node_id != kWeek01NodeId) {
        out_error = {.path = path, .line = 0, .message = "Week-01 pack must use node_id \"club_week_01\"."};
        return std::nullopt;
    }

    std::unordered_map<std::string, std::string> scene_text;
    scene_text.reserve(pack.scene_text.size());
    for (const SceneTextEntry& entry : pack.scene_text) {
        if (entry.key.empty() || entry.text.empty()) {
            out_error = {.path = path, .line = 0, .message = "scene_text entries must set both key and text."};
            return std::nullopt;
        }
        if (!parse_scene_key(entry.key).has_value()) {
            out_error = {.path = path, .line = 0, .message = "Unknown SceneKey: " + entry.key};
            return std::nullopt;
        }
        if (!scene_text.emplace(entry.key, entry.text).second) {
            out_error = {.path = path, .line = 0, .message = "Duplicate scene_text key: " + entry.key};
            return std::nullopt;
        }
    }
    for (const SceneKeyMapping& mapping : kWeek01SceneKeys) {
        const auto it = scene_text.find(std::string {mapping.name});
        if (it == scene_text.end() || it->second.empty()) {
            out_error = {.path = path, .line = 0, .message = "Missing required scene_text key: " + std::string {mapping.name}};
            return std::nullopt;
        }
    }

    std::unordered_map<std::string, MatchStartFeedbackEntry> feedback;
    feedback.reserve(pack.match_start_feedback.size());
    for (const MatchStartFeedbackEntry& entry : pack.match_start_feedback) {
        if (entry.match_kind.empty() || entry.line_1.empty() || entry.line_2.empty()) {
            out_error = {.path = path, .line = 0, .message = "match_start_feedback entries must set match_kind, line_1, and line_2."};
            return std::nullopt;
        }
        if (!parse_week01_feedback_match_kind(entry.match_kind).has_value()) {
            out_error = {.path = path, .line = 0, .message = "Unknown match_start_feedback match_kind: " + entry.match_kind};
            return std::nullopt;
        }
        if (!feedback.emplace(entry.match_kind, entry).second) {
            out_error = {.path = path, .line = 0, .message = "Duplicate match_start_feedback match_kind: " + entry.match_kind};
            return std::nullopt;
        }
    }
    for (const MatchKindMapping& required : kWeek01FeedbackKinds) {
        if (!feedback.contains(std::string {required.name})) {
            out_error = {.path = path, .line = 0, .message = "Missing required match_start_feedback match_kind: " + std::string {required.name}};
            return std::nullopt;
        }
    }

    return pack;
}

std::optional<Season1GraphPack> parse_season1_graph_pack(const fs::path& path, ParseError& out_error) {
    std::ifstream input(path);
    if (!input.is_open()) {
        out_error = {.path = path, .line = 0, .message = "Failed to open file."};
        return std::nullopt;
    }

    Season1GraphPack pack {};
    pack.source_path = path;

    enum class ActiveTable : std::uint8_t {
        None = 0,
        HubNode = 1,
    };
    ActiveTable table = ActiveTable::None;
    HubNodeEntry* active_node = nullptr;

    std::string raw;
    int line = 0;
    while (std::getline(input, raw)) {
        ++line;
        const std::string uncommented = strip_toml_comment(raw);
        const std::string_view trimmed = trim(uncommented);
        if (trimmed.empty()) {
            continue;
        }

        if (trimmed.size() >= 4 && trimmed.starts_with("[[") && trimmed.ends_with("]]")) {
            const std::string_view table_name = trim(trimmed.substr(2, trimmed.size() - 4));
            active_node = nullptr;
            if (table_name == "hub_node") {
                pack.hub_nodes.push_back(HubNodeEntry {});
                active_node = &pack.hub_nodes.back();
                table = ActiveTable::HubNode;
                continue;
            }
            out_error = {.path = path, .line = line, .message = std::string {"Unknown table: "} + std::string {table_name}};
            return std::nullopt;
        }

        const std::size_t eq = trimmed.find('=');
        if (eq == std::string_view::npos) {
            out_error = {.path = path, .line = line, .message = "Expected key = value assignment."};
            return std::nullopt;
        }
        const std::string_view key = trim(trimmed.substr(0, eq));
        const std::string_view value = trim(trimmed.substr(eq + 1));
        if (key.empty()) {
            out_error = {.path = path, .line = line, .message = "Empty key in assignment."};
            return std::nullopt;
        }

        if (table == ActiveTable::None) {
            if (key == "type") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                pack.type = *parsed;
            } else if (key == "pack_id") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                pack.pack_id = *parsed;
            } else if (key == "season_id") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                pack.season_id = *parsed;
            } else {
                out_error = {.path = path, .line = line, .message = std::string {"Unknown top-level key: "} + std::string {key}};
                return std::nullopt;
            }
            continue;
        }

        if (table == ActiveTable::HubNode) {
            if (active_node == nullptr) {
                out_error = {.path = path, .line = line, .message = "Internal parser error (no active hub_node)."};
                return std::nullopt;
            }
            if (key == "node_id") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_node->node_id = *parsed;
            } else if (key == "display_week") {
                const auto parsed = parse_toml_int(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_node->display_week = *parsed;
            } else if (key == "training_rival_id") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                const auto rival_id = parse_story_rival_id(*parsed);
                if (!rival_id.has_value()) {
                    out_error = {.path = path, .line = line, .message = "Unknown training_rival_id: " + *parsed};
                    return std::nullopt;
                }
                active_node->training_rival_id = *rival_id;
            } else if (key == "official_rival_id") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                const auto rival_id = parse_story_rival_id(*parsed);
                if (!rival_id.has_value()) {
                    out_error = {.path = path, .line = line, .message = "Unknown official_rival_id: " + *parsed};
                    return std::nullopt;
                }
                active_node->official_rival_id = *rival_id;
            } else if (key == "next_node_id") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_node->next_node_id = *parsed;
            } else {
                out_error = {.path = path, .line = line, .message = std::string {"Unknown hub_node key: "} + std::string {key}};
                return std::nullopt;
            }
            continue;
        }
    }

    if (pack.type != kExpectedSeasonGraphType) {
        out_error = {.path = path, .line = 0, .message = "Invalid type: expected \"" +
            std::string(kExpectedSeasonGraphType) + "\""};
        return std::nullopt;
    }
    if (pack.pack_id.empty()) {
        out_error = {.path = path, .line = 0, .message = "Missing required top-level key: pack_id"};
        return std::nullopt;
    }
    if (pack.season_id != kExpectedSeason1Id) {
        out_error = {.path = path, .line = 0, .message = "Invalid season_id: expected \"" +
            std::string(kExpectedSeason1Id) + "\""};
        return std::nullopt;
    }
    if (pack.hub_nodes.empty()) {
        out_error = {.path = path, .line = 0, .message = "Season graph pack must define at least one [[hub_node]]."};
        return std::nullopt;
    }

    std::unordered_map<std::string, std::size_t> node_ids;
    node_ids.reserve(pack.hub_nodes.size());
    for (std::size_t i = 0; i < pack.hub_nodes.size(); ++i) {
        const HubNodeEntry& entry = pack.hub_nodes[i];
        if (entry.node_id.empty()) {
            out_error = {.path = path, .line = 0, .message = "hub_node entries must set node_id."};
            return std::nullopt;
        }
        if (entry.display_week < 1) {
            out_error = {.path = path, .line = 0, .message = "hub_node display_week must be >= 1 for node_id: " + entry.node_id};
            return std::nullopt;
        }
        if (!node_ids.emplace(entry.node_id, i).second) {
            out_error = {.path = path, .line = 0, .message = "Duplicate hub_node node_id: " + entry.node_id};
            return std::nullopt;
        }
    }

    if (pack.hub_nodes.front().node_id != kWeek01NodeId) {
        out_error = {.path = path, .line = 0, .message = "Season 1 graph start node must be: " + std::string {kWeek01NodeId}};
        return std::nullopt;
    }

    for (const HubNodeEntry& entry : pack.hub_nodes) {
        if (!entry.next_node_id.empty() && !node_ids.contains(entry.next_node_id)) {
            out_error = {.path = path, .line = 0, .message = "hub_node next_node_id does not resolve: " + entry.next_node_id};
            return std::nullopt;
        }
    }

    return pack;
}

std::optional<MenuStickerPack> parse_menu_sticker_pack(const fs::path& path, ParseError& out_error) {
    std::ifstream input(path);
    if (!input.is_open()) {
        out_error = {.path = path, .line = 0, .message = "Failed to open file."};
        return std::nullopt;
    }

    MenuStickerPack pack {};
    pack.source_path = path;

    enum class ActiveTable : std::uint8_t {
        None = 0,
        Surface = 1,
        Slot = 2,
    };
    ActiveTable table = ActiveTable::None;
    MenuStickerSurfaceEntry* active_surface = nullptr;
    MenuStickerSlotEntry* active_slot = nullptr;

    std::string raw;
    int line = 0;
    while (std::getline(input, raw)) {
        ++line;
        const std::string uncommented = strip_toml_comment(raw);
        const std::string_view trimmed = trim(uncommented);
        if (trimmed.empty()) {
            continue;
        }

        if (trimmed.size() >= 4 && trimmed.starts_with("[[") && trimmed.ends_with("]]")) {
            const std::string_view table_name = trim(trimmed.substr(2, trimmed.size() - 4));
            active_slot = nullptr;
            if (table_name == "menu_sticker_surface") {
                pack.surfaces.push_back(MenuStickerSurfaceEntry {});
                active_surface = &pack.surfaces.back();
                table = ActiveTable::Surface;
                continue;
            }
            if (table_name == "menu_sticker_surface.slot") {
                if (active_surface == nullptr) {
                    out_error = {.path = path, .line = line, .message = "menu_sticker_surface.slot must follow [[menu_sticker_surface]]."};
                    return std::nullopt;
                }
                active_surface->slots.push_back(MenuStickerSlotEntry {});
                active_slot = &active_surface->slots.back();
                table = ActiveTable::Slot;
                continue;
            }
            out_error = {.path = path, .line = line, .message = std::string {"Unknown table: "} + std::string {table_name}};
            return std::nullopt;
        }

        const std::size_t eq = trimmed.find('=');
        if (eq == std::string_view::npos) {
            out_error = {.path = path, .line = line, .message = "Expected key = value assignment."};
            return std::nullopt;
        }
        const std::string_view key = trim(trimmed.substr(0, eq));
        const std::string_view value = trim(trimmed.substr(eq + 1));
        if (key.empty()) {
            out_error = {.path = path, .line = line, .message = "Empty key in assignment."};
            return std::nullopt;
        }

        if (table == ActiveTable::None) {
            if (key == "type") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                pack.type = *parsed;
            } else if (key == "pack_id") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                pack.pack_id = *parsed;
            } else {
                out_error = {.path = path, .line = line, .message = std::string {"Unknown top-level key: "} + std::string {key}};
                return std::nullopt;
            }
            continue;
        }

        if (table == ActiveTable::Surface) {
            if (active_surface == nullptr) {
                out_error = {.path = path, .line = line, .message = "Internal parser error (no active menu_sticker_surface)."};
                return std::nullopt;
            }
            if (key == "surface") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                const auto surface = parse_menu_sticker_surface(*parsed);
                if (!surface.has_value()) {
                    out_error = {.path = path, .line = line, .message = "Unknown menu_sticker_surface value: " + *parsed};
                    return std::nullopt;
                }
                active_surface->surface = *surface;
                active_surface->has_surface = true;
            } else {
                out_error = {.path = path, .line = line, .message = std::string {"Unknown menu_sticker_surface key: "} + std::string {key}};
                return std::nullopt;
            }
            continue;
        }

        if (table == ActiveTable::Slot) {
            if (active_slot == nullptr) {
                out_error = {.path = path, .line = line, .message = "Internal parser error (no active menu_sticker_surface.slot)."};
                return std::nullopt;
            }
            if (key == "asset") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->asset_filename = *parsed;
                active_slot->has_asset = true;
            } else if (key == "anchor") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                const auto anchor = parse_menu_sticker_anchor(*parsed);
                if (!anchor.has_value()) {
                    out_error = {.path = path, .line = line, .message = "Unknown menu_sticker_surface.slot anchor: " + *parsed};
                    return std::nullopt;
                }
                active_slot->anchor = *anchor;
                active_slot->has_anchor = true;
            } else if (key == "placement") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                const auto placement = parse_menu_sticker_placement(*parsed);
                if (!placement.has_value()) {
                    out_error = {.path = path, .line = line, .message = "Unknown menu_sticker_surface.slot placement: " + *parsed};
                    return std::nullopt;
                }
                active_slot->placement = *placement;
            } else if (key == "x_norm") {
                const auto parsed = parse_toml_float(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->x_norm = *parsed;
                active_slot->has_x_norm = true;
            } else if (key == "y_norm") {
                const auto parsed = parse_toml_float(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->y_norm = *parsed;
                active_slot->has_y_norm = true;
            } else if (key == "size_mode") {
                const auto parsed = parse_toml_string(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                const auto size_mode = parse_menu_sticker_size_mode(*parsed);
                if (!size_mode.has_value()) {
                    out_error = {.path = path, .line = line, .message = "Unknown menu_sticker_surface.slot size_mode: " + *parsed};
                    return std::nullopt;
                }
                active_slot->size_mode = *size_mode;
            } else if (key == "scale") {
                const auto parsed = parse_toml_float(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->scale = *parsed;
                active_slot->has_scale = true;
            } else if (key == "height_px") {
                const auto parsed = parse_toml_float(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->height_px = *parsed;
                active_slot->has_height_px = true;
            } else if (key == "offset_x_px") {
                const auto parsed = parse_toml_float(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->offset_x_px = *parsed;
            } else if (key == "offset_y_px") {
                const auto parsed = parse_toml_float(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->offset_y_px = *parsed;
            } else if (key == "z_order") {
                const auto parsed = parse_toml_int(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->z_order = *parsed;
            } else if (key == "rotation_deg") {
                const auto parsed = parse_toml_float(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->rotation_deg = *parsed;
            } else if (key == "allow_protected_overlap") {
                const auto parsed = parse_toml_bool(path, line, value, out_error);
                if (!parsed) {
                    return std::nullopt;
                }
                active_slot->allow_protected_overlap = *parsed;
            } else {
                out_error = {.path = path, .line = line, .message = std::string {"Unknown menu_sticker_surface.slot key: "} + std::string {key}};
                return std::nullopt;
            }
            continue;
        }
    }

    if (pack.type != kExpectedMenuStickersType) {
        out_error = {.path = path, .line = 0, .message = "Invalid type: expected \"" +
            std::string(kExpectedMenuStickersType) + "\""};
        return std::nullopt;
    }
    if (pack.pack_id.empty()) {
        out_error = {.path = path, .line = 0, .message = "Missing required top-level key: pack_id"};
        return std::nullopt;
    }
    if (pack.surfaces.empty()) {
        out_error = {.path = path, .line = 0, .message = "Menu sticker pack must define at least one [[menu_sticker_surface]]."};
        return std::nullopt;
    }

    std::unordered_map<int, std::size_t> seen_surfaces;
    seen_surfaces.reserve(pack.surfaces.size());
    for (std::size_t i = 0; i < pack.surfaces.size(); ++i) {
        MenuStickerSurfaceEntry& surface = pack.surfaces[i];
        if (!surface.has_surface) {
            out_error = {.path = path, .line = 0, .message = "menu_sticker_surface entries must set surface."};
            return std::nullopt;
        }
        const int surface_key = static_cast<int>(surface.surface);
        if (!seen_surfaces.emplace(surface_key, i).second) {
            out_error = {.path = path, .line = 0, .message = "Duplicate menu_sticker_surface for surface value."};
            return std::nullopt;
        }
        if (surface.slots.size() < 1 || surface.slots.size() > 6) {
            out_error = {.path = path, .line = 0, .message = "menu_sticker_surface must define 1 to 6 slot entries."};
            return std::nullopt;
        }
        for (MenuStickerSlotEntry& slot : surface.slots) {
            if (!slot.has_asset || slot.asset_filename.empty()) {
                out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot entries must set asset."};
                return std::nullopt;
            }
            if (!slot.asset_filename.ends_with(".png")) {
                out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot asset must end with .png: " + slot.asset_filename};
                return std::nullopt;
            }
            if (slot.placement == MenuStickerPlacement::Edge) {
                if (!slot.has_anchor) {
                    out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot entries must set anchor when placement is edge."};
                    return std::nullopt;
                }
            } else {
                if (!slot.has_x_norm || !slot.has_y_norm) {
                    out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot placement=inside requires x_norm and y_norm."};
                    return std::nullopt;
                }
                if (slot.x_norm < 0.0f || slot.x_norm > 1.0f || slot.y_norm < 0.0f || slot.y_norm > 1.0f) {
                    out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot x_norm and y_norm must be in [0.0, 1.0]."};
                    return std::nullopt;
                }
            }
            if (slot.size_mode == MenuStickerSizeMode::Scale) {
                if (!slot.has_scale) {
                    out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot size_mode=scale requires scale."};
                    return std::nullopt;
                }
                if (slot.scale < 0.05f || slot.scale > 0.60f) {
                    out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot scale must be in [0.05, 0.60]."};
                    return std::nullopt;
                }
            } else {
                if (!slot.has_height_px) {
                    out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot size_mode=height_px requires height_px."};
                    return std::nullopt;
                }
                if (slot.height_px < 24.0f || slot.height_px > 260.0f) {
                    out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot height_px must be in [24, 260]."};
                    return std::nullopt;
                }
            }
            if (slot.rotation_deg < -20.0f || slot.rotation_deg > 20.0f) {
                out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot rotation_deg must be in [-20, 20]."};
                return std::nullopt;
            }
            if (slot.z_order < 0 || slot.z_order > 10) {
                out_error = {.path = path, .line = 0, .message = "menu_sticker_surface.slot z_order must be in [0, 10]."};
                return std::nullopt;
            }
        }
    }

    return pack;
}

std::optional<std::string> render_story_pack_generated_cpp(
    const Week01TextPack& week01_pack,
    const Season1GraphPack& season1_graph,
    const MenuStickerPack& menu_stickers,
    ParseError& out_error) {
    std::unordered_map<std::string, std::string> scene_text;
    scene_text.reserve(week01_pack.scene_text.size());
    for (const SceneTextEntry& entry : week01_pack.scene_text) {
        scene_text.emplace(entry.key, entry.text);
    }

    std::unordered_map<std::string, MatchStartFeedbackEntry> feedback;
    feedback.reserve(week01_pack.match_start_feedback.size());
    for (const MatchStartFeedbackEntry& entry : week01_pack.match_start_feedback) {
        feedback.emplace(entry.match_kind, entry);
    }

    std::ostringstream out;
    out << "// Generated by story_pack_compiler from:\n";
    out << "// - " << week01_pack.source_path.string() << "\n";
    out << "// - " << season1_graph.source_path.string() << "\n";
    out << "// - " << menu_stickers.source_path.string() << "\n";
    out << "#include <array>\n\n";
    out << "#include \"story_pack.hpp\"\n";
    out << "#include \"menu_sticker_pack.hpp\"\n";
    out << "#include \"story_rivals.hpp\"\n";
    out << "#include \"story_script_catalog.hpp\"\n\n";
    out << "namespace whacker::app::story_pack {\n\n";
    out << "std::string_view week_01_scene_text(const story_text_week::SceneKey key) {\n";
    out << "    switch (key) {\n";
    for (const SceneKeyMapping& mapping : kWeek01SceneKeys) {
        const auto it = scene_text.find(std::string {mapping.name});
        if (it == scene_text.end()) {
            out_error = {.path = week01_pack.source_path, .line = 0, .message = "Missing required scene_text key during render: " + std::string {mapping.name}};
            return std::nullopt;
        }
        out << "        case story_text_week::SceneKey::" << mapping.name << ":\n";
        out << "            return \"" << escape_cpp_string_literal(it->second) << "\";\n";
    }
    out << "    }\n";
    out << "    return {};\n";
    out << "}\n\n";

    out << "bool week_01_match_start_feedback(const StoryMatchKind match_kind, story_text::FeedbackLines& out_feedback) {\n";
    out << "    switch (match_kind) {\n";
    for (const MatchKindMapping& required : kWeek01FeedbackKinds) {
        const auto it = feedback.find(std::string {required.name});
        if (it == feedback.end()) {
            out_error = {.path = week01_pack.source_path, .line = 0, .message = "Missing required match_start_feedback during render: " + std::string {required.name}};
            return std::nullopt;
        }
        out << "        case StoryMatchKind::" << required.name << ":\n";
        out << "            out_feedback = {\n";
        out << "                .line_1 = \"" << escape_cpp_string_literal(it->second.line_1) << "\",\n";
        out << "                .line_2 = \"" << escape_cpp_string_literal(it->second.line_2) << "\"\n";
        out << "            };\n";
        out << "            return true;\n";
    }
    out << "        case StoryMatchKind::OnboardingAyaFriendly:\n";
    out << "        case StoryMatchKind::OnboardingEntry:\n";
    out << "        case StoryMatchKind::None:\n";
    out << "        default:\n";
    out << "            return false;\n";
    out << "    }\n";
    out << "}\n\n";

    out << "std::span<const StoryGraphNodeSpec> season1_hub_graph() {\n";
    out << "    static const std::array<StoryGraphNodeSpec, " << season1_graph.hub_nodes.size() << "> graph = {{\n";
    for (const HubNodeEntry& entry : season1_graph.hub_nodes) {
        out << "        StoryGraphNodeSpec {\n";
        out << "            .node_id = \"" << escape_cpp_string_literal(entry.node_id) << "\",\n";
        out << "            .kind = StoryGraphNodeKind::Hub,\n";
        out << "            .display_week = " << entry.display_week << ",\n";
        out << "            .training_rival = story_rival_spec(StoryRivalId::" << story_rival_id_cpp_name(entry.training_rival_id) << "),\n";
        out << "            .official_rival = story_rival_spec(StoryRivalId::" << story_rival_id_cpp_name(entry.official_rival_id) << "),\n";
        if (entry.next_node_id.empty()) {
            out << "            .next_node_id = {},\n";
        } else {
            out << "            .next_node_id = \"" << escape_cpp_string_literal(entry.next_node_id) << "\",\n";
        }
        out << "        },\n";
    }
    out << "    }};\n";
    out << "    return graph;\n";
    out << "}\n\n";

    out << "std::span<const MenuStickerSurfaceSpec> menu_sticker_surfaces() {\n";
    for (const MenuStickerSurfaceEntry& surface : menu_stickers.surfaces) {
        out << "    static const std::array<MenuStickerSlotSpec, " << surface.slots.size() << "> slots_"
            << menu_sticker_surface_cpp_name(surface.surface) << " = {{\n";
        for (const MenuStickerSlotEntry& slot : surface.slots) {
            out << "        MenuStickerSlotSpec {\n";
            out << "            .asset_filename = \"" << escape_cpp_string_literal(slot.asset_filename) << "\",\n";
            out << "            .anchor = MenuStickerAnchor::" << menu_sticker_anchor_cpp_name(slot.anchor) << ",\n";
            out << "            .placement = MenuStickerPlacement::" << menu_sticker_placement_cpp_name(slot.placement) << ",\n";
            out << "            .x_norm = " << format_cpp_float_literal(slot.x_norm) << ",\n";
            out << "            .y_norm = " << format_cpp_float_literal(slot.y_norm) << ",\n";
            out << "            .size_mode = MenuStickerSizeMode::" << menu_sticker_size_mode_cpp_name(slot.size_mode) << ",\n";
            out << "            .scale = " << format_cpp_float_literal(slot.scale) << ",\n";
            out << "            .height_px = " << format_cpp_float_literal(slot.height_px) << ",\n";
            out << "            .offset_x_px = " << format_cpp_float_literal(slot.offset_x_px) << ",\n";
            out << "            .offset_y_px = " << format_cpp_float_literal(slot.offset_y_px) << ",\n";
            out << "            .rotation_deg = " << format_cpp_float_literal(slot.rotation_deg) << ",\n";
            out << "            .allow_protected_overlap = " << (slot.allow_protected_overlap ? "true" : "false") << ",\n";
            out << "            .z_order = " << slot.z_order << ",\n";
            out << "        },\n";
        }
        out << "    }};\n";
    }
    out << "    static const std::array<MenuStickerSurfaceSpec, " << menu_stickers.surfaces.size() << "> surfaces = {{\n";
    for (const MenuStickerSurfaceEntry& surface : menu_stickers.surfaces) {
        out << "        MenuStickerSurfaceSpec {\n";
        out << "            .surface = MenuStickerSurface::" << menu_sticker_surface_cpp_name(surface.surface) << ",\n";
        out << "            .slots = slots_" << menu_sticker_surface_cpp_name(surface.surface) << ",\n";
        out << "        },\n";
    }
    out << "    }};\n";
    out << "    return surfaces;\n";
    out << "}\n\n";

    out << "}  // namespace whacker::app::story_pack\n";

    return out.str();
}

int usage() {
    std::cerr << "Usage: story_pack_compiler --input <pack.toml> [--input <pack.toml> ...] --out-cpp <path> [--validate-only]\n";
    return 2;
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<fs::path> input_paths;
    fs::path out_cpp_path;
    bool validate_only = false;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg {argv[i]};
        if (arg == "--validate-only") {
            validate_only = true;
            continue;
        }
        if (arg == "--input" && i + 1 < argc) {
            input_paths.push_back(fs::path {argv[++i]});
            continue;
        }
        if (arg == "--out-cpp" && i + 1 < argc) {
            out_cpp_path = fs::path {argv[++i]};
            continue;
        }
        return usage();
    }

    if (input_paths.empty()) {
        return usage();
    }
    if (!validate_only && out_cpp_path.empty()) {
        return usage();
    }

    ParseError error {};
    std::optional<Week01TextPack> week01_pack;
    std::optional<Season1GraphPack> season1_graph;
    std::optional<MenuStickerPack> menu_stickers;
    for (const fs::path& input_path : input_paths) {
        const auto pack_type = parse_pack_type(input_path, error);
        if (!pack_type) {
            std::cerr << format_error(error) << "\n";
            return 1;
        }

        if (*pack_type == kExpectedType) {
            if (week01_pack.has_value()) {
                std::cerr << input_path.string() << ": duplicate week_text pack input.\n";
                return 1;
            }
            const auto parsed = parse_week01_text_pack(input_path, error);
            if (!parsed) {
                std::cerr << format_error(error) << "\n";
                return 1;
            }
            week01_pack = *parsed;
            continue;
        }

        if (*pack_type == kExpectedSeasonGraphType) {
            if (season1_graph.has_value()) {
                std::cerr << input_path.string() << ": duplicate season_graph pack input.\n";
                return 1;
            }
            const auto parsed = parse_season1_graph_pack(input_path, error);
            if (!parsed) {
                std::cerr << format_error(error) << "\n";
                return 1;
            }
            season1_graph = *parsed;
            continue;
        }

        if (*pack_type == kExpectedMenuStickersType) {
            if (menu_stickers.has_value()) {
                std::cerr << input_path.string() << ": duplicate menu_stickers pack input.\n";
                return 1;
            }
            const auto parsed = parse_menu_sticker_pack(input_path, error);
            if (!parsed) {
                std::cerr << format_error(error) << "\n";
                return 1;
            }
            menu_stickers = *parsed;
            continue;
        }

        std::cerr << input_path.string() << ": unknown pack type: " << *pack_type << "\n";
        return 1;
    }

    if (!week01_pack.has_value()) {
        std::cerr << "Missing required pack input: type = \"" << kExpectedType << "\"\n";
        return 1;
    }
    if (!season1_graph.has_value()) {
        std::cerr << "Missing required pack input: type = \"" << kExpectedSeasonGraphType << "\"\n";
        return 1;
    }
    if (!menu_stickers.has_value()) {
        std::cerr << "Missing required pack input: type = \"" << kExpectedMenuStickersType << "\"\n";
        return 1;
    }

    if (validate_only) {
        return 0;
    }

    const auto rendered = render_story_pack_generated_cpp(*week01_pack, *season1_graph, *menu_stickers, error);
    if (!rendered) {
        std::cerr << format_error(error) << "\n";
        return 1;
    }

    std::error_code mkdir_error;
    fs::create_directories(out_cpp_path.parent_path(), mkdir_error);
    if (mkdir_error) {
        std::cerr << out_cpp_path.string() << ": failed to create output directory: " << mkdir_error.message() << "\n";
        return 1;
    }

    std::ofstream output(out_cpp_path);
    if (!output.is_open()) {
        std::cerr << out_cpp_path.string() << ": failed to open output file\n";
        return 1;
    }
    output << *rendered;
    output.flush();
    if (!output.good()) {
        std::cerr << out_cpp_path.string() << ": failed while writing output file\n";
        return 1;
    }

    return 0;
}
