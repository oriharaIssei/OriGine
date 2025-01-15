#include "GameDeltaTime.h"

void GameDeltaTime::Init() {
    DeltaTime::Init();
}

void GameDeltaTime::Update() {
    DeltaTime::Update();

    if (timeScalingTime_ > 0.0f) {
        timeScalingTime_ -= deltaTime_;
        if (timeScalingTime_ <= 0.0f) {
            timeScale_       = 1.0f;
            timeScalingTime_ = -1.0f;
        }
    }

    deltaTime_ *= timeScale_;
}
