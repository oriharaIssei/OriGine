#include "IdleAction.h"

///engine
#include "Engine.h"

///application
//enemy
#include "application/Enemy/IEnemy.h"

namespace EnemyBehavior {
IdleAction::IdleAction(float _time)
    : leftTime_(_time) {}

IdleAction::~IdleAction() {}

Status IdleAction::tick() {
    leftTime_ -= enemy_->DeltaTime();
    if (leftTime_ <= 0.0f) {
        return Status::SUCCESS;
    }
    return Status::RUNNING;
}
} // namespace EnemyBehavior
