#pragma once
#include "DeltaTime.h"

class GameDeltaTime {
public:
    void Init();
    void Update();

private:
    DeltaTime* deltaTime_ = nullptr;

    float timeScale_ = 1.0f;
    float timeScalingTime_ = -1.0f;

    float scaledDeltaTime_ = 0.0f;

public:
    void HitStop(float _timeScale, float _timeScalingTime) {
        timeScale_       = _timeScale;
        timeScalingTime_ = _timeScalingTime;
    }

    void setTimeScale(float timeScale) { timeScale_ = timeScale; }
    float getTimeScale() const { return timeScale_; }

    void setTimeScalingTime(float timeScalingTime) { timeScalingTime_ = timeScalingTime; }
    float getTimeScalingTime() const { return timeScalingTime_; }

    float getDeltaTime() const { return scaledDeltaTime_; }
};
