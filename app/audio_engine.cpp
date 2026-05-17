#include "audio_engine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <memory>
#include <vector>

#ifdef WHACKER_HAS_SDL2_AUDIO
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#endif

namespace whacker::app {

namespace {

constexpr float kPi = 3.14159265358979323846f;

float volume_01(const int value_0_to_100) {
    return static_cast<float>(std::clamp(value_0_to_100, 0, 100)) * 0.01f;
}

}  // namespace

AudioSettings clamp_audio_settings(AudioSettings settings) {
    settings.master_volume = std::clamp(settings.master_volume, 0, 100);
    settings.music_volume = std::clamp(settings.music_volume, 0, 100);
    settings.sfx_volume = std::clamp(settings.sfx_volume, 0, 100);
    return settings;
}

#ifdef WHACKER_HAS_SDL2_AUDIO

namespace {

enum class Waveform : std::uint8_t {
    Sine = 0,
    Square = 1,
    Noise = 2
};

struct Voice {
    bool active = false;
    Waveform waveform = Waveform::Sine;
    float frequency_hz = 440.0f;
    float frequency_delta_hz_per_s = 0.0f;
    float phase = 0.0f;
    float amplitude = 0.2f;
    float age_seconds = 0.0f;
    float duration_seconds = 0.1f;
    float delay_seconds = 0.0f;
    float attack_seconds = 0.003f;
    float decay_seconds = 0.03f;
    float sustain_level = 0.4f;
    float release_seconds = 0.04f;
    std::uint32_t noise_state = 0x12345678u;
};

struct QueuedAudioEvent {
    AudioEventId id = AudioEventId::None;
    float p0 = 0.0f;
    float p1 = 0.0f;
    float p2 = 0.0f;
    float p3 = 0.0f;
};

float envelope_amplitude(const Voice& voice) {
    const float t = voice.age_seconds;
    const float attack = std::max(voice.attack_seconds, 1.0e-4f);
    const float decay = std::max(voice.decay_seconds, 1.0e-4f);
    const float release = std::max(voice.release_seconds, 1.0e-4f);
    const float sustain_level = std::clamp(voice.sustain_level, 0.0f, 1.0f);
    const float sustain_end = std::max(attack + decay, voice.duration_seconds - release);

    if (t < 0.0f) {
        return 0.0f;
    }
    if (t < attack) {
        return t / attack;
    }
    if (t < attack + decay) {
        const float d = (t - attack) / decay;
        return 1.0f + (sustain_level - 1.0f) * d;
    }
    if (t < sustain_end) {
        return sustain_level;
    }
    if (t < voice.duration_seconds) {
        const float r = (t - sustain_end) / release;
        return sustain_level * std::max(0.0f, 1.0f - r);
    }
    return 0.0f;
}

float next_noise(std::uint32_t& state) {
    state = state * 1664525u + 1013904223u;
    const float normalized = static_cast<float>((state >> 8) & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
    return normalized * 2.0f - 1.0f;
}

Voice make_voice(
    const Waveform waveform,
    const float frequency_hz,
    const float amplitude,
    const float duration_seconds,
    const float attack_seconds,
    const float decay_seconds,
    const float sustain_level,
    const float release_seconds,
    const float delay_seconds = 0.0f,
    const float frequency_delta_hz_per_s = 0.0f) {
    Voice voice {};
    voice.active = true;
    voice.waveform = waveform;
    voice.frequency_hz = frequency_hz;
    voice.frequency_delta_hz_per_s = frequency_delta_hz_per_s;
    voice.phase = 0.0f;
    voice.amplitude = amplitude;
    voice.age_seconds = 0.0f;
    voice.duration_seconds = std::max(duration_seconds, 0.01f);
    voice.delay_seconds = std::max(delay_seconds, 0.0f);
    voice.attack_seconds = std::max(attack_seconds, 1.0e-4f);
    voice.decay_seconds = std::max(decay_seconds, 1.0e-4f);
    voice.sustain_level = std::clamp(sustain_level, 0.0f, 1.0f);
    voice.release_seconds = std::max(release_seconds, 1.0e-4f);
    voice.noise_state = 0x9E3779B9u;
    return voice;
}

}  // namespace

struct AudioEngine::Impl {
    SDL_AudioDeviceID device = 0;
    SDL_AudioSpec obtained_spec {};
    AudioSettings settings {};
    std::uint32_t type_blip_counter = 0;

    static constexpr std::size_t kMaxEvents = 256;
    std::array<QueuedAudioEvent, kMaxEvents> events {};
    std::size_t event_head = 0;
    std::size_t event_tail = 0;

    static constexpr std::size_t kMaxVoices = 32;
    std::array<Voice, kMaxVoices> voices {};

    float music_phase_1 = 0.0f;
    float music_phase_2 = 0.0f;
    float music_phase_3 = 0.0f;

    void push_event_unlocked(
        const AudioEventId event_id,
        const float p0 = 0.0f,
        const float p1 = 0.0f,
        const float p2 = 0.0f,
        const float p3 = 0.0f) {
        const std::size_t next_tail = (event_tail + 1u) % kMaxEvents;
        if (next_tail == event_head) {
            return;
        }
        events[event_tail] = QueuedAudioEvent {.id = event_id, .p0 = p0, .p1 = p1, .p2 = p2, .p3 = p3};
        event_tail = next_tail;
    }

    bool pop_event_unlocked(QueuedAudioEvent& out_event) {
        if (event_head == event_tail) {
            return false;
        }
        out_event = events[event_head];
        event_head = (event_head + 1u) % kMaxEvents;
        return true;
    }

    void spawn_voice_unlocked(const Voice& voice) {
        for (Voice& slot : voices) {
            if (!slot.active) {
                slot = voice;
                return;
            }
        }
        voices.front() = voice;
    }

    void spawn_event_voices_unlocked(const QueuedAudioEvent& event) {
        switch (event.id) {
            case AudioEventId::PaddleHit: {
                const float power = std::clamp(event.p0, 0.0f, 1.0f);
                const float angle = std::clamp(event.p1, 0.0f, 1.0f);
                const float spin = std::clamp(event.p2, 0.0f, 1.0f);
                const float spin_sign = std::clamp(event.p3, -1.0f, 1.0f);

                // Impact body: lower and heavier as power rises.
                const float body_freq = 545.0f - (300.0f * power) - (40.0f * angle);
                const float spin_body_duck = 1.0f - (0.45f * spin);
                const float body_amp = (0.10f + (0.18f * power)) * spin_body_duck;
                const float body_dur = (0.042f + (0.050f * power)) * (1.0f - (0.28f * spin));
                spawn_voice_unlocked(make_voice(
                    Waveform::Square,
                    body_freq,
                    body_amp,
                    body_dur,
                    0.0008f,
                    0.013f,
                    0.16f,
                    0.016f));

                // Low thunk under power shots so they read as forceful, not bright.
                spawn_voice_unlocked(make_voice(
                    Waveform::Sine,
                    210.0f - (75.0f * power),
                    (0.028f + (0.100f * power)) * (1.0f - (0.70f * spin)),
                    (0.055f + (0.070f * power)) * (1.0f - (0.35f * spin)),
                    0.0007f,
                    0.011f,
                    0.12f,
                    0.020f,
                    0.0006f));

                const float angle_tick_freq = 1040.0f + (1450.0f * angle);
                const float angle_tick_amp = (0.008f + (0.050f * angle)) * (1.0f - (0.45f * power));
                spawn_voice_unlocked(make_voice(
                    Waveform::Sine,
                    angle_tick_freq,
                    angle_tick_amp,
                    0.016f,
                    0.0006f,
                    0.0048f,
                    0.08f,
                    0.006f,
                    0.0007f));

                if (spin > 0.004f) {
                    const bool up_zip = spin_sign >= 0.0f;
                    const float zip_start = up_zip ? (1550.0f + (300.0f * spin)) : (4300.0f + (850.0f * spin));
                    const float zip_end = up_zip ? (5000.0f + (1200.0f * spin)) : (950.0f + (260.0f * spin));
                    const float zip_duration = 0.024f + (0.010f * spin);
                    const float zip_delta = (zip_end - zip_start) / std::max(zip_duration, 1.0e-3f);
                    const float zip_amp = 0.018f + (0.070f * spin);
                    // Continuous directional glide for a true "zip" read.
                    spawn_voice_unlocked(make_voice(
                        Waveform::Sine,
                        zip_start,
                        zip_amp,
                        zip_duration,
                        0.00035f,
                        0.0036f,
                        0.03f,
                        0.0036f,
                        0.0012f,
                        zip_delta));
                    spawn_voice_unlocked(make_voice(
                        Waveform::Sine,
                        zip_start * 1.32f,
                        zip_amp * 0.42f,
                        zip_duration * 0.92f,
                        0.00035f,
                        0.0032f,
                        0.02f,
                        0.0030f,
                        0.0020f,
                        zip_delta * 1.16f));
                    spawn_voice_unlocked(make_voice(
                        Waveform::Noise,
                        0.0f,
                        0.0052f + (0.012f * spin),
                        0.016f + (0.006f * spin),
                        0.00025f,
                        0.0022f,
                        0.03f,
                        0.0024f,
                        0.0013f));
                    // Tiny high click for bite.
                    spawn_voice_unlocked(make_voice(
                        Waveform::Square,
                        3300.0f + (900.0f * spin),
                        0.0026f + (0.0054f * spin),
                        0.008f,
                        0.0002f,
                        0.0015f,
                        0.02f,
                        0.0018f,
                        0.0030f));
                }
                break;
            }
            case AudioEventId::WallHit: {
                const float impact = std::clamp(event.p0, 0.0f, 1.0f);
                const float spin = std::clamp(event.p1, 0.0f, 1.0f);
                const float spin_sign = std::clamp(event.p2, -1.0f, 1.0f);

                const float body_freq = 980.0f + (210.0f * impact);
                const float body_amp = 0.080f + (0.070f * impact);
                const float body_dur = 0.016f + (0.012f * impact);
                spawn_voice_unlocked(make_voice(
                    Waveform::Square,
                    body_freq,
                    body_amp,
                    body_dur,
                    0.0006f,
                    0.0050f,
                    0.09f,
                    0.007f));
                spawn_voice_unlocked(make_voice(
                    Waveform::Sine,
                    1380.0f + (300.0f * impact),
                    0.024f + (0.030f * impact),
                    0.015f,
                    0.0005f,
                    0.0042f,
                    0.07f,
                    0.006f,
                    0.0008f));

                if (spin > 0.02f) {
                    const bool up_zip = spin_sign >= 0.0f;
                    const float zip_start = up_zip ? (1450.0f + (240.0f * spin)) : (3550.0f + (540.0f * spin));
                    const float zip_end = up_zip ? (3950.0f + (720.0f * spin)) : (980.0f + (220.0f * spin));
                    const float zip_duration = 0.019f + (0.008f * spin);
                    const float zip_delta = (zip_end - zip_start) / std::max(zip_duration, 1.0e-3f);
                    const float zip_amp = 0.011f + (0.046f * spin);
                    spawn_voice_unlocked(make_voice(
                        Waveform::Sine,
                        zip_start,
                        zip_amp,
                        zip_duration,
                        0.0003f,
                        0.0030f,
                        0.03f,
                        0.0030f,
                        0.0012f,
                        zip_delta));
                    spawn_voice_unlocked(make_voice(
                        Waveform::Sine,
                        zip_start * 1.28f,
                        zip_amp * 0.38f,
                        zip_duration * 0.88f,
                        0.0003f,
                        0.0023f,
                        0.02f,
                        0.0022f,
                        0.0020f,
                        zip_delta * 1.12f));
                    spawn_voice_unlocked(make_voice(
                        Waveform::Noise,
                        0.0f,
                        0.0034f + (0.0074f * spin),
                        0.012f,
                        0.0002f,
                        0.0018f,
                        0.03f,
                        0.0020f,
                        0.0011f));
                }
                break;
            }
            case AudioEventId::Score:
                spawn_voice_unlocked(make_voice(Waveform::Sine, 660.0f, 0.24f, 0.10f, 0.003f, 0.03f, 0.4f, 0.04f));
                spawn_voice_unlocked(make_voice(Waveform::Sine, 880.0f, 0.20f, 0.10f, 0.003f, 0.03f, 0.35f, 0.04f, 0.06f));
                break;
            case AudioEventId::MenuMove:
                spawn_voice_unlocked(make_voice(Waveform::Sine, 700.0f, 0.08f, 0.03f, 0.001f, 0.01f, 0.15f, 0.01f));
                break;
            case AudioEventId::MenuConfirm:
                spawn_voice_unlocked(make_voice(Waveform::Sine, 520.0f, 0.12f, 0.05f, 0.002f, 0.02f, 0.3f, 0.02f));
                spawn_voice_unlocked(make_voice(Waveform::Sine, 780.0f, 0.10f, 0.05f, 0.002f, 0.02f, 0.25f, 0.02f, 0.03f));
                break;
            case AudioEventId::ServeBlink:
                spawn_voice_unlocked(make_voice(Waveform::Sine, 980.0f, 0.060f, 0.030f, 0.0006f, 0.006f, 0.10f, 0.008f));
                spawn_voice_unlocked(make_voice(Waveform::Sine, 1320.0f, 0.026f, 0.022f, 0.0006f, 0.005f, 0.08f, 0.007f, 0.0010f));
                break;
            case AudioEventId::TypeBlip:
            {
                // Non-uniform low keyboard chatter with variable release clack.
                constexpr std::array<float, 8> kBodyHz = {405.0f, 360.0f, 385.0f, 335.0f, 430.0f, 375.0f, 350.0f, 415.0f};
                constexpr std::array<float, 8> kAirHz = {610.0f, 560.0f, 590.0f, 535.0f, 650.0f, 575.0f, 550.0f, 620.0f};
                const std::uint32_t step = type_blip_counter++;
                const std::size_t slot = static_cast<std::size_t>(step % kBodyHz.size());
                std::uint32_t hash = step * 1664525u + 1013904223u;
                auto next_rand_01 = [&hash]() -> float {
                    hash = hash * 1664525u + 1013904223u;
                    return static_cast<float>((hash >> 8) & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
                };
                const float r1 = next_rand_01();
                const float r2 = next_rand_01();
                const float r3 = next_rand_01();
                const bool accent = (r2 > 0.80f) || (step % 9u == 4u);
                const float tone_gain = accent ? 1.16f : (0.84f + 0.30f * r1);
                const float clack_gain = accent ? 1.85f : (0.90f + 0.38f * r3);
                const float release_delay = 0.017f + 0.010f * r2 + (accent ? 0.004f : 0.0f);
                const float body_hz = kBodyHz[slot] * (0.92f + 0.18f * r1);
                const float air_hz = kAirHz[slot] * (0.92f + 0.16f * r3);

                spawn_voice_unlocked(make_voice(
                    Waveform::Sine,
                    body_hz,
                    0.022f * tone_gain,
                    0.031f,
                    0.0027f,
                    0.0085f,
                    0.14f,
                    0.0065f));
                spawn_voice_unlocked(make_voice(
                    Waveform::Sine,
                    air_hz,
                    0.0044f * tone_gain,
                    0.019f,
                    0.0022f,
                    0.0065f,
                    0.10f,
                    0.0058f,
                    0.0034f));

                // Irregular ghost body pulse to break uniform cadence.
                if (r1 > 0.56f) {
                    spawn_voice_unlocked(make_voice(
                        Waveform::Sine,
                        body_hz * (0.88f + 0.10f * r3),
                        0.0060f * tone_gain,
                        0.018f,
                        0.0020f,
                        0.0055f,
                        0.08f,
                        0.0050f,
                        0.005f + 0.006f * r2));
                }

                // Key-down clack (deeper/rounder than previous high tick).
                spawn_voice_unlocked(make_voice(
                    Waveform::Square,
                    980.0f,
                    0.0078f * clack_gain,
                    0.014f,
                    0.0002f,
                    0.0020f,
                    0.02f,
                    0.0028f));
                spawn_voice_unlocked(make_voice(
                    Waveform::Noise,
                    0.0f,
                    0.0038f * clack_gain,
                    0.012f,
                    0.0002f,
                    0.0018f,
                    0.02f,
                    0.0024f,
                    0.0003f));

                // Key-up release clack with longer audible tail.
                spawn_voice_unlocked(make_voice(
                    Waveform::Square,
                    1480.0f,
                    0.0088f * clack_gain,
                    0.022f,
                    0.0002f,
                    0.0021f,
                    0.02f,
                    0.0042f,
                    release_delay));
                spawn_voice_unlocked(make_voice(
                    Waveform::Square,
                    2260.0f,
                    0.0038f * clack_gain,
                    0.016f,
                    0.0002f,
                    0.0017f,
                    0.02f,
                    0.0032f,
                    release_delay + 0.0012f));
                spawn_voice_unlocked(make_voice(
                    Waveform::Noise,
                    0.0f,
                    0.0036f * clack_gain,
                    0.015f,
                    0.0002f,
                    0.0016f,
                    0.02f,
                    0.0032f,
                    release_delay + 0.0007f));

                // Accent peak: occasional extra dry knock.
                if (accent) {
                    spawn_voice_unlocked(make_voice(
                        Waveform::Square,
                        860.0f + 120.0f * r1,
                        0.0060f,
                        0.014f,
                        0.0002f,
                        0.0019f,
                        0.02f,
                        0.0030f,
                        release_delay + 0.0020f));
                }
                break;
            }
            case AudioEventId::None:
            default:
                break;
        }
    }

    static void sdl_callback(void* userdata, std::uint8_t* stream, int len) {
        Impl* self = static_cast<Impl*>(userdata);
        if (self == nullptr || stream == nullptr || len <= 0) {
            return;
        }
        if (self->obtained_spec.channels < 1 || self->obtained_spec.freq <= 0) {
            std::fill(stream, stream + len, 0);
            return;
        }
        if (self->obtained_spec.format == AUDIO_F32SYS) {
            const int sample_count = len / static_cast<int>(sizeof(float));
            auto* out = reinterpret_cast<float*>(stream);
            std::fill(out, out + sample_count, 0.0f);
            self->render_into_buffer(out, sample_count);
            return;
        }
        if (self->obtained_spec.format == AUDIO_S16SYS) {
            const int sample_count = len / static_cast<int>(sizeof(std::int16_t));
            thread_local std::vector<float> mix_buffer;
            mix_buffer.assign(static_cast<std::size_t>(sample_count), 0.0f);
            self->render_into_buffer(mix_buffer.data(), sample_count);
            auto* out = reinterpret_cast<std::int16_t*>(stream);
            for (int i = 0; i < sample_count; ++i) {
                const float clamped = std::clamp(mix_buffer[static_cast<std::size_t>(i)], -1.0f, 1.0f);
                out[i] = static_cast<std::int16_t>(std::lrint(clamped * 32767.0f));
            }
            return;
        }

        std::fill(stream, stream + len, 0);
    }

    void render_into_buffer(float* out, const int sample_count) {
        QueuedAudioEvent event {};
        while (pop_event_unlocked(event)) {
            spawn_event_voices_unlocked(event);
        }

        const int channels = std::max(1, static_cast<int>(obtained_spec.channels));
        const int frames = sample_count / channels;
        if (frames <= 0) {
            return;
        }

        const float sample_rate = static_cast<float>(obtained_spec.freq);
        const float dt = 1.0f / sample_rate;

        const AudioSettings clamped = clamp_audio_settings(settings);
        const float master = clamped.mute ? 0.0f : volume_01(clamped.master_volume);
        const float music_gain = master * volume_01(clamped.music_volume) * 0.20f;
        const float sfx_gain = master * volume_01(clamped.sfx_volume);

        for (int frame = 0; frame < frames; ++frame) {
            float sfx_mix = 0.0f;
            for (Voice& voice : voices) {
                if (!voice.active) {
                    continue;
                }
                if (voice.delay_seconds > 0.0f) {
                    voice.delay_seconds -= dt;
                    continue;
                }

                // Advance age BEFORE envelope eval so the first sample
                // is at t=dt (not t=0 where the attack ramp is exactly 0
                // and the voice would be immediately culled).
                voice.age_seconds += dt;

                const float env = envelope_amplitude(voice);
                if (voice.age_seconds > voice.duration_seconds && env <= 1.0e-5f) {
                    voice.active = false;
                    continue;
                }

                float sample = 0.0f;
                switch (voice.waveform) {
                    case Waveform::Sine:
                        sample = std::sin(voice.phase);
                        break;
                    case Waveform::Square:
                        sample = std::sin(voice.phase) >= 0.0f ? 1.0f : -1.0f;
                        break;
                    case Waveform::Noise:
                        sample = next_noise(voice.noise_state);
                        break;
                }
                sfx_mix += sample * voice.amplitude * env;

                const float current_frequency_hz = std::clamp(voice.frequency_hz, 20.0f, 16000.0f);
                voice.phase += 2.0f * kPi * current_frequency_hz * dt;
                voice.frequency_hz = std::clamp(
                    voice.frequency_hz + (voice.frequency_delta_hz_per_s * dt),
                    20.0f,
                    16000.0f);
                if (voice.phase > 2.0f * kPi) {
                    voice.phase = std::fmod(voice.phase, 2.0f * kPi);
                }
            }

            music_phase_1 += 2.0f * kPi * 110.0f * dt;
            music_phase_2 += 2.0f * kPi * 146.832f * dt;
            music_phase_3 += 2.0f * kPi * 174.614f * dt;
            if (music_phase_1 > 2.0f * kPi) {
                music_phase_1 -= 2.0f * kPi;
            }
            if (music_phase_2 > 2.0f * kPi) {
                music_phase_2 -= 2.0f * kPi;
            }
            if (music_phase_3 > 2.0f * kPi) {
                music_phase_3 -= 2.0f * kPi;
            }
            const float music_mix =
                std::sin(music_phase_1) * 0.55f +
                std::sin(music_phase_2) * 0.30f +
                std::sin(music_phase_3) * 0.25f;

            const float mixed = std::clamp(music_mix * music_gain + sfx_mix * sfx_gain, -0.95f, 0.95f);
            for (int c = 0; c < channels; ++c) {
                out[frame * channels + c] = mixed;
            }
        }
    }
};

#else

struct AudioEngine::Impl {
    AudioSettings settings {};
};

#endif

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::init() {
    shutdown();
    impl_ = new Impl {};
    impl_->settings = clamp_audio_settings(impl_->settings);

#ifdef WHACKER_HAS_SDL2_AUDIO
    // Give the stream a stable app/stream identity so session managers do not
    // bucket us under generic "SDL Application" state.
#ifdef SDL_HINT_AUDIO_DEVICE_APP_NAME
    (void)SDL_SetHint(SDL_HINT_AUDIO_DEVICE_APP_NAME, "Whacker");
#endif
#ifdef SDL_HINT_AUDIO_DEVICE_STREAM_NAME
    (void)SDL_SetHint(SDL_HINT_AUDIO_DEVICE_STREAM_NAME, "Whacker");
#endif
#ifdef SDL_HINT_APP_NAME
    (void)SDL_SetHint(SDL_HINT_APP_NAME, "Whacker");
#endif

    const auto init_audio_subsystem = []() -> bool {
        if ((SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) != 0u) {
            return true;
        }
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0) {
            return true;
        }
        std::fprintf(
            stderr,
            "Audio init failed: SDL_InitSubSystem(SDL_INIT_AUDIO): %s\n",
            SDL_GetError());
        return false;
    };

    const auto open_audio_device = [&]() -> bool {
        SDL_AudioSpec desired {};
        desired.freq = 48000;
        desired.format = AUDIO_F32SYS;
        desired.channels = 2;
        desired.samples = 512;
        desired.callback = &Impl::sdl_callback;
        desired.userdata = impl_;
        constexpr int kAllowChanges =
            SDL_AUDIO_ALLOW_FREQUENCY_CHANGE |
            SDL_AUDIO_ALLOW_CHANNELS_CHANGE |
            SDL_AUDIO_ALLOW_FORMAT_CHANGE;

        constexpr std::array<int, 2> kFreqs {48000, 44100};
        constexpr std::array<SDL_AudioFormat, 2> kFormats {AUDIO_F32SYS, AUDIO_S16SYS};

        // Route through the session default device and let PipeWire/WirePlumber
        // own sink selection. This keeps behavior aligned with desktop routing.
        for (const SDL_AudioFormat format : kFormats) {
            desired.format = format;
            for (const int freq : kFreqs) {
                desired.freq = freq;
                impl_->device = SDL_OpenAudioDevice(nullptr, 0, &desired, &impl_->obtained_spec, kAllowChanges);
                if (impl_->device != 0) {
                    return true;
                }
            }
        }
        return false;
    };

    const auto close_audio_subsystem = [&]() {
        if (impl_->device != 0) {
            SDL_CloseAudioDevice(impl_->device);
            impl_->device = 0;
        }
        if ((SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) != 0u) {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        }
    };

    const auto is_dummy_driver = []() -> bool {
        const char* driver = SDL_GetCurrentAudioDriver();
        return driver != nullptr && std::strcmp(driver, "dummy") == 0;
    };

    const char* env_driver = SDL_getenv("SDL_AUDIODRIVER");
    const bool user_forced_driver = env_driver != nullptr && env_driver[0] != '\0';
    bool opened_audio = false;

    if (user_forced_driver) {
        if (init_audio_subsystem() && open_audio_device()) {
            opened_audio = true;
        } else {
            std::fprintf(
                stderr,
                "Audio init failed for SDL_AUDIODRIVER=%s: %s\n",
                env_driver,
                SDL_GetError());
            close_audio_subsystem();
        }
    } else {
#if defined(__linux__)
        // Prefer modern Linux session backends by default.
        // Do not auto-fallback to ALSA here; ALSA may block on some setups.
        constexpr std::array<const char*, 2> kPreferredDrivers {"pipewire", "pulseaudio"};
#elif defined(_WIN32)
        constexpr std::array<const char*, 3> kPreferredDrivers {"wasapi", "directsound", "winmm"};
#elif defined(__APPLE__)
        constexpr std::array<const char*, 1> kPreferredDrivers {"coreaudio"};
#else
        constexpr std::array<const char*, 0> kPreferredDrivers {};
#endif
        for (const char* driver : kPreferredDrivers) {
            if (driver == nullptr || driver[0] == '\0') {
                continue;
            }
            close_audio_subsystem();
            (void)SDL_setenv("SDL_AUDIODRIVER", driver, 1);
            if (!init_audio_subsystem()) {
                continue;
            }
            if (!open_audio_device()) {
                close_audio_subsystem();
                continue;
            }
            if (is_dummy_driver()) {
                close_audio_subsystem();
                continue;
            }
            opened_audio = true;
            break;
        }

        if (!opened_audio) {
            std::fprintf(
                stderr,
                "Audio init failed: no preferred audio backend available. "
                "Try SDL_AUDIODRIVER=pipewire or SDL_AUDIODRIVER=pulseaudio.\n");
        }
    }

    if (!opened_audio) {
        close_audio_subsystem();
        delete impl_;
        impl_ = nullptr;
        return false;
    }

    if (is_dummy_driver()) {
        std::fprintf(
            stderr,
            "Audio init failed: SDL selected dummy driver (silent). "
            "Set SDL_AUDIODRIVER explicitly.\n");
        close_audio_subsystem();
        delete impl_;
        impl_ = nullptr;
        return false;
    }

    if (impl_->obtained_spec.format != AUDIO_F32SYS && impl_->obtained_spec.format != AUDIO_S16SYS) {
        std::fprintf(
            stderr,
            "Audio init failed: unsupported obtained format (got 0x%x, expected float32 or s16)\n",
            static_cast<unsigned>(impl_->obtained_spec.format));
        close_audio_subsystem();
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    SDL_PauseAudioDevice(impl_->device, 0);
    return true;
#else
    std::fprintf(stderr, "Audio disabled at build time: SDL2 backend not enabled.\n");
    return false;
#endif
}

void AudioEngine::shutdown() {
    if (impl_ == nullptr) {
        return;
    }
#ifdef WHACKER_HAS_SDL2_AUDIO
    if (impl_->device != 0) {
        SDL_CloseAudioDevice(impl_->device);
        impl_->device = 0;
    }
    if ((SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) != 0u) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
#endif
    delete impl_;
    impl_ = nullptr;
}

bool AudioEngine::available() const {
    if (impl_ == nullptr) {
        return false;
    }
#ifdef WHACKER_HAS_SDL2_AUDIO
    return impl_->device != 0;
#else
    return false;
#endif
}

void AudioEngine::set_settings(const AudioSettings& settings) {
    if (impl_ == nullptr) {
        return;
    }
    const AudioSettings clamped = clamp_audio_settings(settings);
#ifdef WHACKER_HAS_SDL2_AUDIO
    if (impl_->device != 0) {
        SDL_LockAudioDevice(impl_->device);
        impl_->settings = clamped;
        SDL_UnlockAudioDevice(impl_->device);
        return;
    }
#endif
    impl_->settings = clamped;
}

AudioSettings AudioEngine::settings() const {
    if (impl_ == nullptr) {
        return {};
    }
    return impl_->settings;
}

void AudioEngine::push_event(const AudioEventId event_id) {
    if (impl_ == nullptr || event_id == AudioEventId::None) {
        return;
    }
#ifdef WHACKER_HAS_SDL2_AUDIO
    if (impl_->device != 0) {
        SDL_LockAudioDevice(impl_->device);
        impl_->push_event_unlocked(event_id, 0.0f, 0.0f, 0.0f, 0.0f);
        SDL_UnlockAudioDevice(impl_->device);
    }
#else
    (void)event_id;
#endif
}

void AudioEngine::push_paddle_hit(const PaddleHitAudioParams& params) {
    if (impl_ == nullptr) {
        return;
    }
#ifdef WHACKER_HAS_SDL2_AUDIO
    if (impl_->device != 0) {
        SDL_LockAudioDevice(impl_->device);
        impl_->push_event_unlocked(
            AudioEventId::PaddleHit,
            std::clamp(params.power, 0.0f, 1.0f),
            std::clamp(params.angle, 0.0f, 1.0f),
            std::clamp(params.spin, 0.0f, 1.0f),
            std::clamp(params.spin_sign, -1.0f, 1.0f));
        SDL_UnlockAudioDevice(impl_->device);
    }
#else
    (void)params;
#endif
}

void AudioEngine::push_wall_hit(const WallHitAudioParams& params) {
    if (impl_ == nullptr) {
        return;
    }
#ifdef WHACKER_HAS_SDL2_AUDIO
    if (impl_->device != 0) {
        SDL_LockAudioDevice(impl_->device);
        impl_->push_event_unlocked(
            AudioEventId::WallHit,
            std::clamp(params.impact, 0.0f, 1.0f),
            std::clamp(params.spin, 0.0f, 1.0f),
            std::clamp(params.spin_sign, -1.0f, 1.0f),
            0.0f);
        SDL_UnlockAudioDevice(impl_->device);
    }
#else
    (void)params;
#endif
}

}  // namespace whacker::app
