#include "GameDeltaTime.h"

///engine
#include "Engine.h"

void GameDeltaTime::Init() {
    deltaTime_ = Engine::getInstance()->getDeltaTimeInstance();
}

void GameDeltaTime::Update() {
    scaledDeltaTime_ = deltaTime_->getDeltaTime();

    if (timeScalingTime_ > 0.0f) {
        // この時点では scaleDeltaTime == deltaTime
        timeScalingTime_ -= scaledDeltaTime_;
        if (timeScalingTime_ <= 0.0f) {
            timeScale_       = 1.0f;
            timeScalingTime_ = -1.0f;
        }
    }

    scaledDeltaTime_ *= timeScale_;
}
