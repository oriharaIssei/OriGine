#include "DeltaTimer.h"

#include <logger/Logger.h>

void DeltaTimer::Initialize() {
    currentTime_ = std::chrono::high_resolution_clock::now();
    preTime_     = currentTime_;

    deltaTime_        = 0.0f;
    totalHistoryTime_ = 0.0f;
    frameHistory_.clear();
}

void DeltaTimer::Update() {
    preTime_     = currentTime_;
    currentTime_ = std::chrono::high_resolution_clock::now();
    deltaTime_   = static_cast<float>(std::chrono::duration<float>(currentTime_ - preTime_).count());

    // --- 平均計測用に追加 ---
    frameHistory_.push_back(deltaTime_);
    totalHistoryTime_ += deltaTime_;
    if (frameHistory_.size() > kMaxHistorySize) {
        totalHistoryTime_ -= frameHistory_.front();
        frameHistory_.pop_front();
    }
}

float DeltaTimer::GetAverageDeltaTime() const {
    if (frameHistory_.empty()) {
        return 0.0f;
    }
    return totalHistoryTime_ / static_cast<float>(frameHistory_.size());
}

float DeltaTimer::GetAverageDeltaTime(size_t _frameCount) const {
    if (frameHistory_.empty() || _frameCount == 0) {
        return 0.0f;
    }
    size_t count = (std::min)(_frameCount, frameHistory_.size());
    float total  = 0.0f;
    for (size_t i = frameHistory_.size() - count; i < frameHistory_.size(); ++i) {
        total += frameHistory_[i];
    }
    return total / static_cast<float>(count);
}

size_t DeltaTimer::GetAverageFPS() const {
    if (frameHistory_.empty()) {
        return 0;
    }
    return static_cast<size_t>(frameHistory_.size() / totalHistoryTime_);
}

size_t DeltaTimer::GetAverageFPS(size_t _frameCount) const {
    if (frameHistory_.empty() || _frameCount == 0) {
        return 0;
    }
    size_t count = (std::min)(_frameCount, frameHistory_.size());
    float total  = 0.0f;
    for (size_t i = frameHistory_.size() - count; i < frameHistory_.size(); ++i) {
        total += frameHistory_[i];
    }
    return static_cast<size_t>(count / total);
}

float DeltaTimer::GetScaledDeltaTime(const std::string& _key) const {
    auto itr = deltaTimeScaleMap_.find(_key);
    if (itr != deltaTimeScaleMap_.end()) {
        return deltaTime_ * itr->second;
    }
    LOG_DEBUG("Key '{}' not found in deltaTimeScaleMap_. Returning unscaled deltaTime.", _key);
    return deltaTime_;
}

void DeltaTimer::SetTimeScale(const std::string& _key, float _scale) {
    deltaTimeScaleMap_[_key] = _scale;
}
