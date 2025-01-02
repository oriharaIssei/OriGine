#include "WeakAttackAction.h"

namespace EnemyBehavior {
WeakAttackAction::WeakAttackAction(float _attack)
    : attack_(_attack) {}

WeakAttackAction::~WeakAttackAction() {}

Status WeakAttackAction::tick() {
    // 攻撃 アニメーションを流すだけ
    if (!isEndMotion_) {
        return Status::RUNNING;
    }
    return Status::SUCCESS;
}
} // namespace EnemyBehavior
