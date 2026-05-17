#include "sim/config_io.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>

namespace whacker::sim {

namespace {

std::string trim(const std::string& value) {
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }

    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return value.substr(start, end - start);
}

bool parse_float(const std::string& value, float* out_value) {
    if (out_value == nullptr) {
        return false;
    }

    char* end = nullptr;
    const float parsed = std::strtof(value.c_str(), &end);
    if (end == value.c_str()) {
        return false;
    }

    while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end)) != 0) {
        ++end;
    }

    if (*end != '\0') {
        return false;
    }
    if (!std::isfinite(parsed)) {
        return false;
    }

    *out_value = parsed;
    return true;
}

void set_error(std::string* error_message, const std::string& message) {
    if (error_message != nullptr) {
        *error_message = message;
    }
}

std::string line_error_message(const char* prefix, const int line_number, const std::string& path) {
    std::string message(prefix);
    message.append(std::to_string(line_number));
    message.append(" in ");
    message.append(path);
    return message;
}

std::string keyed_line_error_message(
    const char* prefix,
    const std::string& key,
    const int line_number,
    const std::string& path) {
    std::string message(prefix);
    message.append(key);
    message.append("' at line ");
    message.append(std::to_string(line_number));
    message.append(" in ");
    message.append(path);
    return message;
}

}  // namespace

bool load_config_from_json_file(const std::string& path, SimulationConfig& config, std::string* error_message) {
    std::ifstream input(path);
    if (!input.is_open()) {
        set_error(error_message, "Could not open config file: " + path);
        return false;
    }

    SimulationConfig parsed = config;
    std::unordered_map<std::string, float*> fields {
        {"court_width", &parsed.court_width},
        {"court_height", &parsed.court_height},
        {"ball_radius", &parsed.ball_radius},
        {"paddle_half_height", &parsed.paddle_half_height},
        {"paddle_half_width", &parsed.paddle_half_width},
        {"paddle_x_margin", &parsed.paddle_x_margin},
        {"paddle_max_speed", &parsed.paddle_max_speed},
        {"paddle_accel", &parsed.paddle_accel},
        {"theta_max_rad", &parsed.theta_max_rad},
        {"spin_max", &parsed.spin_max},
        {"k_take", &parsed.k_take},
        {"k_curve", &parsed.k_curve},
        {"curve_speed_exponent", &parsed.curve_speed_exponent},
        {"tau_spin", &parsed.tau_spin},
        {"spin_burn_speed_gain", &parsed.spin_burn_speed_gain},
        {"tau_speed", &parsed.tau_speed},
        {"wall_spin_retention", &parsed.wall_spin_retention},
        {"k_wall_spin_tangent", &parsed.k_wall_spin_tangent},
        {"k_spin_contact_bias", &parsed.k_spin_contact_bias},
        {"k_spin_rebound", &parsed.k_spin_rebound},
        {"paddle_spin_retention", &parsed.paddle_spin_retention},
        {"ball_base_speed", &parsed.ball_base_speed},
        {"ball_speed_scalar_cap", &parsed.ball_speed_scalar_cap},
        {"ramp_rate", &parsed.ramp_rate},
        {"power_contact_boost", &parsed.power_contact_boost},
        {"power_to_spin_coupling", &parsed.power_to_spin_coupling},
        {"power_to_technical_coupling", &parsed.power_to_technical_coupling},
        {"power_energy_spin_drag", &parsed.power_energy_spin_drag},
        {"power_energy_technical_drag", &parsed.power_energy_technical_drag},
        {"power_spin_transfer_ratio", &parsed.power_spin_transfer_ratio},
        {"spin_counter_power_ratio", &parsed.spin_counter_power_ratio},
        {"spin_counter_power_cap", &parsed.spin_counter_power_cap},
    };

    std::string line;
    int line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        const std::string clean = trim(line);
        if (clean.empty() || clean == "{" || clean == "}") {
            continue;
        }

        const std::size_t key_begin = clean.find('"');
        if (key_begin == std::string::npos) {
            continue;
        }

        const std::size_t key_end = clean.find('"', key_begin + 1);
        if (key_end == std::string::npos) {
            set_error(error_message, line_error_message("Malformed key at line ", line_number, path));
            return false;
        }

        const std::string key = clean.substr(key_begin + 1, key_end - key_begin - 1);
        const std::size_t colon = clean.find(':', key_end + 1);
        if (colon == std::string::npos) {
            set_error(
                error_message,
                keyed_line_error_message("Missing ':' for key '", key, line_number, path));
            return false;
        }

        std::string value_text = trim(clean.substr(colon + 1));
        if (!value_text.empty() && value_text.back() == ',') {
            value_text.pop_back();
            value_text = trim(value_text);
        }

        auto field = fields.find(key);
        if (field == fields.end()) {
            continue;
        }

        float parsed_value = 0.0f;
        if (!parse_float(value_text, &parsed_value)) {
            set_error(
                error_message,
                keyed_line_error_message("Invalid float for key '", key, line_number, path));
            return false;
        }

        *(field->second) = parsed_value;
    }

    config = parsed;
    if (error_message != nullptr) {
        error_message->clear();
    }
    return true;
}

}  // namespace whacker::sim
