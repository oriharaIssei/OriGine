#include "IdleAction.h"

#include "Engine.h"

namespace EnemyBehavior {
IdleAction::IdleAction(float _time)
    : leftTime_(_time) {}

IdleAction::~IdleAction() {}

Status IdleAction::tick() {
    leftTime_ -= Engine::getInstance()->getDeltaTime();
    if (leftTime_ <= 0.0f) {
        return Status::SUCCESS;
    }
    return Status::RUNNING;
}
} // namespace EnemyBehavior
