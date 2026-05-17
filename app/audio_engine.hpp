#pragma once

#include <cstdint>

namespace whacker::app {

enum class AudioEventId : std::uint8_t {
    None = 0,
    PaddleHit = 1,
    WallHit = 2,
    Score = 3,
    MenuMove = 4,
    MenuConfirm = 5,
    TypeBlip = 6,
    ServeBlink = 7
};

struct PaddleHitAudioParams {
    float power = 0.0f;      // 0..1
    float angle = 0.0f;      // 0..1
    float spin = 0.0f;       // 0..1
    float spin_sign = 0.0f;  // -1..1
};

struct WallHitAudioParams {
    float impact = 0.0f;     // 0..1
    float spin = 0.0f;       // 0..1
    float spin_sign = 0.0f;  // -1..1
};

struct AudioSettings {
    int master_volume = 80;  // 0..100
    int music_volume = 45;   // 0..100
    int sfx_volume = 70;     // 0..100
    bool mute = false;
};

class AudioEngine {
public:
    AudioEngine() = default;
    ~AudioEngine();

    bool init();
    void shutdown();

    bool available() const;

    void set_settings(const AudioSettings& settings);
    AudioSettings settings() const;

    void push_event(AudioEventId event_id);
    void push_paddle_hit(const PaddleHitAudioParams& params);
    void push_wall_hit(const WallHitAudioParams& params);

private:
    struct Impl;
    Impl* impl_ = nullptr;
};

AudioSettings clamp_audio_settings(AudioSettings settings);

}  // namespace whacker::app
