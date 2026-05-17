#include "menu_settings.hpp"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#ifdef WHACKER_HAS_GLFW

#include <GLFW/glfw3.h>

#include "ai_style_catalog.hpp"
#include "paddle_tuning.hpp"
#include "text_utils.hpp"

namespace {

std::vector<std::string> menu_settings_paths() {
    return {"config/menu_settings.cfg", "../config/menu_settings.cfg"};
}

bool parse_int_value(const std::string& text, int& out_value) {
    if (text.empty()) {
        return false;
    }
    std::size_t idx = 0;
    try {
        const long parsed = std::stol(text, &idx, 10);
        if (idx != text.size()) {
            return false;
        }
        out_value = static_cast<int>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_float_value(const std::string& text, float& out_value) {
    if (text.empty()) {
        return false;
    }
    std::size_t idx = 0;
    try {
        const float parsed = std::stof(text, &idx);
        if (idx != text.size()) {
            return false;
        }
        if (!std::isfinite(parsed)) {
            return false;
        }
        out_value = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_mode(const std::string& value, whacker::app::PaddleMode& mode_out) {
    if (value == "Human" || value == "human" || value == "0") {
        mode_out = whacker::app::PaddleMode::Human;
        return true;
    }
    if (value == "AI" || value == "ai" || value == "1") {
        mode_out = whacker::app::PaddleMode::AI;
        return true;
    }
    return false;
}

const char* mode_name(const whacker::app::PaddleMode mode) {
    return mode == whacker::app::PaddleMode::Human ? "Human" : "AI";
}

}  // namespace

namespace whacker::app {

void load_menu_settings(MatchOptions& options, ControlBindings& controls, AudioSettings& audio_settings) {
    for (const std::string& path : menu_settings_paths()) {
        std::ifstream input(path);
        if (!input.is_open()) {
            continue;
        }

        MatchOptions loaded_options = options;
        ControlBindings loaded_controls = controls;
        AudioSettings loaded_audio = audio_settings;
        bool loaded_left_skills = false;
        bool loaded_right_skills = false;
        std::string line;
        while (std::getline(input, line)) {
            const std::string clean = trim_copy(line);
            if (clean.empty() || clean[0] == '#') {
                continue;
            }

            const std::size_t eq = clean.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            const std::string key = trim_copy(clean.substr(0, eq));
            const std::string value = trim_copy(clean.substr(eq + 1));

            if (key == "left_mode") {
                (void)parse_mode(value, loaded_options.left_mode);
            } else if (key == "right_mode") {
                (void)parse_mode(value, loaded_options.right_mode);
            } else if (key == "left_ai_style") {
                (void)whacker::app::parse_ai_style(value, loaded_options.left_ai_style);
            } else if (key == "right_ai_style") {
                (void)whacker::app::parse_ai_style(value, loaded_options.right_ai_style);
            } else if (key == "left_skill_edge") {
                loaded_left_skills = true;
                (void)parse_float_value(value, loaded_options.left_paddle_skills.edge);
            } else if (key == "left_skill_power") {
                loaded_left_skills = true;
                (void)parse_float_value(value, loaded_options.left_paddle_skills.power);
            } else if (key == "left_skill_spin") {
                loaded_left_skills = true;
                (void)parse_float_value(value, loaded_options.left_paddle_skills.spin_inject);
            } else if (key == "right_skill_edge") {
                loaded_right_skills = true;
                (void)parse_float_value(value, loaded_options.right_paddle_skills.edge);
            } else if (key == "right_skill_power") {
                loaded_right_skills = true;
                (void)parse_float_value(value, loaded_options.right_paddle_skills.power);
            } else if (key == "right_skill_spin") {
                loaded_right_skills = true;
                (void)parse_float_value(value, loaded_options.right_paddle_skills.spin_inject);
            } else if (key == "p1_up_key") {
                int parsed = GLFW_KEY_UNKNOWN;
                if (parse_int_value(value, parsed) && is_bindable_key(parsed)) {
                    loaded_controls.p1_up = parsed;
                }
            } else if (key == "p1_down_key") {
                int parsed = GLFW_KEY_UNKNOWN;
                if (parse_int_value(value, parsed) && is_bindable_key(parsed)) {
                    loaded_controls.p1_down = parsed;
                }
            } else if (key == "p2_up_key") {
                int parsed = GLFW_KEY_UNKNOWN;
                if (parse_int_value(value, parsed) && is_bindable_key(parsed)) {
                    loaded_controls.p2_up = parsed;
                }
            } else if (key == "p2_down_key") {
                int parsed = GLFW_KEY_UNKNOWN;
                if (parse_int_value(value, parsed) && is_bindable_key(parsed)) {
                    loaded_controls.p2_down = parsed;
                }
            } else if (key == "master_volume") {
                int parsed = loaded_audio.master_volume;
                if (parse_int_value(value, parsed)) {
                    loaded_audio.master_volume = parsed;
                }
            } else if (key == "music_volume") {
                int parsed = loaded_audio.music_volume;
                if (parse_int_value(value, parsed)) {
                    loaded_audio.music_volume = parsed;
                }
            } else if (key == "sfx_volume") {
                int parsed = loaded_audio.sfx_volume;
                if (parse_int_value(value, parsed)) {
                    loaded_audio.sfx_volume = parsed;
                }
            } else if (key == "mute") {
                loaded_audio.mute = (value == "1" || value == "true" || value == "True" || value == "on");
            }
        }

        if (!loaded_left_skills) {
            loaded_options.left_paddle_skills = whacker::app::ai_style_profile(loaded_options.left_ai_style).seed_skills;
        } else {
            whacker::progression::clamp_skills(loaded_options.left_paddle_skills);
        }
        if (!loaded_right_skills) {
            loaded_options.right_paddle_skills = whacker::app::ai_style_profile(loaded_options.right_ai_style).seed_skills;
        } else {
            whacker::progression::clamp_skills(loaded_options.right_paddle_skills);
        }
        loaded_options.left_ai_style = style_for_skills(loaded_options.left_paddle_skills);
        loaded_options.right_ai_style = style_for_skills(loaded_options.right_paddle_skills);
        options = loaded_options;
        controls = loaded_controls;
        audio_settings = clamp_audio_settings(loaded_audio);
        std::printf("Loaded menu settings: %s\n", path.c_str());
        return;
    }
}

void save_menu_settings(
    const MatchOptions& options,
    const ControlBindings& controls,
    const AudioSettings& audio_settings) {
    for (const std::string& path : menu_settings_paths()) {
        std::ofstream output(path);
        if (!output.is_open()) {
            continue;
        }

        output << "# Whacker menu settings\n";
        output << "left_mode=" << mode_name(options.left_mode) << "\n";
        output << "right_mode=" << mode_name(options.right_mode) << "\n";
        output << "left_ai_style=" << whacker::app::ai_style_name(style_for_skills(options.left_paddle_skills)) << "\n";
        output << "right_ai_style=" << whacker::app::ai_style_name(style_for_skills(options.right_paddle_skills)) << "\n";
        output << "left_skill_edge=" << options.left_paddle_skills.edge << "\n";
        output << "left_skill_power=" << options.left_paddle_skills.power << "\n";
        output << "left_skill_spin=" << options.left_paddle_skills.spin_inject << "\n";
        output << "right_skill_edge=" << options.right_paddle_skills.edge << "\n";
        output << "right_skill_power=" << options.right_paddle_skills.power << "\n";
        output << "right_skill_spin=" << options.right_paddle_skills.spin_inject << "\n";
        output << "p1_up_key=" << controls.p1_up << "\n";
        output << "p1_down_key=" << controls.p1_down << "\n";
        output << "p2_up_key=" << controls.p2_up << "\n";
        output << "p2_down_key=" << controls.p2_down << "\n";
        const AudioSettings clamped = clamp_audio_settings(audio_settings);
        output << "master_volume=" << clamped.master_volume << "\n";
        output << "music_volume=" << clamped.music_volume << "\n";
        output << "sfx_volume=" << clamped.sfx_volume << "\n";
        output << "mute=" << (clamped.mute ? "1" : "0") << "\n";
        return;
    }
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
