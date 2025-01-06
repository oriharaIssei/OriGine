#include "KnockBack.h"

#include "Engine.h"
// parentObject
#include "../IEnemy.h"

namespace EnemyBehavior {
KnockBackAction::KnockBackAction(const Vector3& _direction, float _speed)
    : velocity_(_direction * _speed) {}

KnockBackAction::~KnockBackAction() {}

const float attenuation = 0.8f; // 速度減衰率
Status KnockBackAction::tick() {
    float deltaTime  = Engine::getInstance()->getDeltaTime();
    Vector3 enemyPos = enemy_->getTranslate();

    // ノックバックの挙動
    velocity_ *= attenuation * deltaTime;

    if (velocity_.lengthSq() <= 0.0f) {
        return Status::SUCCESS;
    }
    enemy_->setTranslate(enemyPos + velocity_ * deltaTime);
    return Status::RUNNING;
}
} // namespace EnemyBehavior
