#include "KnockBack.h"

#include "Engine.h"
// parentObject
#include "../IEnemy.h"

namespace EnemyBehavior {
KnockBackAction::KnockBackAction(const Vec3f& _direction, float _speed, std::unique_ptr<Node> _originalBehavior)
    : velocity_(_direction * _speed), originalBehavior_(std::move(_originalBehavior)) {}

KnockBackAction::~KnockBackAction() {}

const float attenuation = 0.8f; // 速度減衰率
Status KnockBackAction::tick() {
    float deltaTime  = Engine::getInstance()->getGameDeltaTime();
    Vec3f enemyPos  = enemy_->getTranslate();

    // ノックバックの挙動
    velocity_ *= attenuation;

    if (velocity_.lengthSq() <= 10.00f) {
        // ノックバックが終了したら元のBehaviorに戻す
        enemy_->setBehaviorTree(originalBehavior_);
        return Status::SUCCESS;
    }
    enemy_->setTranslate(enemyPos + velocity_ * deltaTime);
    return Status::RUNNING;
}
} // namespace EnemyBehavior
