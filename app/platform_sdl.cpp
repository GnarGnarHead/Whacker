#include "platform_sdl.hpp"

#ifndef WHACKER_PLATFORM_SDL2
#error "platform_sdl.cpp must be compiled with WHACKER_PLATFORM_SDL2"
#endif

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

namespace whacker::app {

namespace {

constexpr Uint32 kOwnedSubsystems =
    SDL_INIT_VIDEO |
    SDL_INIT_EVENTS |
    SDL_INIT_TIMER |
    SDL_INIT_GAMECONTROLLER;

}  // namespace

SdlPlatform::~SdlPlatform() {
    shutdown();
}

bool SdlPlatform::init(std::string* error_message) {
    if (initialized_) {
        return true;
    }

    SDL_SetMainReady();
#ifdef SDL_HINT_APP_NAME
    (void)SDL_SetHint(SDL_HINT_APP_NAME, "Whacker");
#endif

    if (SDL_Init(kOwnedSubsystems) != 0) {
        if (error_message != nullptr) {
            *error_message = SDL_GetError();
        }
        return false;
    }

    initialized_ = true;
    return true;
}

void SdlPlatform::shutdown() {
    if (!initialized_) {
        return;
    }
    SDL_QuitSubSystem(kOwnedSubsystems);
    initialized_ = false;
}

bool SdlPlatform::initialized() const {
    return initialized_;
}

double SdlPlatform::now_seconds() const {
    return static_cast<double>(SDL_GetTicks64()) / 1000.0;
}

}  // namespace whacker::app
