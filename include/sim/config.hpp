#pragma once

namespace whacker::sim {

constexpr float kFixedDt = 1.0f / 240.0f;

struct SimulationConfig {
    float court_width = 640.0f;
    float court_height = 360.0f;
    float ball_radius = 4.0f;

    float paddle_half_height = 26.0f;
    float paddle_half_width = 4.0f;
    float paddle_x_margin = 24.0f;
    float paddle_max_speed = 420.0f;
    float paddle_accel = 5200.0f;

    float theta_max_rad = 1.05f;

    float spin_max = 7.0f;
    float k_take = 0.026f;
    float k_curve = 195.0f;
    // Speed exponent applied to in-flight spin curve response.
    float curve_speed_exponent = 1.40f;
    float tau_spin = 4.2f;
    // Extra spin burn applied at speeds above ball_base_speed.
    float spin_burn_speed_gain = 1.00f;
    float tau_speed = 16.0f;
    float wall_spin_retention = 0.996f;
    float k_wall_spin_tangent = 16.0f;
    float k_spin_contact_bias = 0.22f;
    float k_spin_rebound = 14.0f;
    float paddle_spin_retention = 0.97f;

    float ball_base_speed = 250.0f;
    float ball_speed_scalar_cap = 6.0f;
    // Legacy tuning term kept for config compatibility. Contact power now requires impact velocity.
    float ramp_rate = 0.022f;
    float power_contact_boost = 27.0f;
    float power_to_spin_coupling = 0.85f;
    float power_to_technical_coupling = 0.55f;
    float power_energy_spin_drag = 0.40f;
    float power_energy_technical_drag = 0.24f;
    // Fraction of excess shot speed converted into additional spin on high-spin contacts.
    float power_spin_transfer_ratio = 0.45f;
    // Fraction of canceled opposite spin that can be recovered as shot speed.
    float spin_counter_power_ratio = 0.20f;
    // Per-contact cap on counter-spin speed gain before terminal-velocity shaping.
    float spin_counter_power_cap = 0.12f;

};

}  // namespace whacker::sim
