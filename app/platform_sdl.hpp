#pragma once

#include <string>

namespace whacker::app {

class SdlPlatform {
public:
    SdlPlatform() = default;
    ~SdlPlatform();

    SdlPlatform(const SdlPlatform&) = delete;
    SdlPlatform& operator=(const SdlPlatform&) = delete;

    bool init(std::string* error_message = nullptr);
    void shutdown();

    bool initialized() const;
    double now_seconds() const;

private:
    bool initialized_ = false;
};

}  // namespace whacker::app
