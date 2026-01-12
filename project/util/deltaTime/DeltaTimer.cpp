#include "DeltaTimer.h"

#include <logger/Logger.h>

void DeltaTimer::Initialize() {
    currentTime_ = std::chrono::high_resolution_clock::now();
    preTime_     = currentTime_;
}

void DeltaTimer::Update() {
    preTime_     = currentTime_;
    currentTime_ = std::chrono::high_resolution_clock::now();
    deltaTime_   = static_cast<float>(std::chrono::duration<float>(currentTime_ - preTime_).count());
}

float DeltaTimer::GetScaledDeltaTime(const std::string& key) const {
    auto itr = deltaTimeScaleMap_.find(key);
    if (itr != deltaTimeScaleMap_.end()) {
        return deltaTime_ * itr->second;
    }
    LOG_DEBUG("Key '{}' not found in deltaTimeScaleMap_. Returning unscaled deltaTime.", key);
    return deltaTime_;
}

void DeltaTimer::SetTimeScale(const std::string& key, float scale) {
    deltaTimeScaleMap_[key] = scale;
}
