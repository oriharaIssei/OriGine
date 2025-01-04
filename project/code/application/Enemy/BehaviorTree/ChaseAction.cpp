#include "ChaseAction.h"

///engine
#include "Engine.h"
//component
#include "../IEnemy.h"
#include "application/Player/Player.h"
//lib
#include "myRandom/MyRandom.h"

namespace EnemyBehavior {
ChaseAction::ChaseAction(float _speed, float _targetDistance)
    : speed_(_speed),
      targetDistance_(_targetDistance) {}

ChaseAction::~ChaseAction() {}

Status ChaseAction::tick() {
    Vector3 playerPos = enemy_->getPlayer()->getTranslate();
    Vector3 enemyPos  = enemy_->getTranslate();
    Vector3 dist      = playerPos - enemyPos;

    // 規定距離内なら 攻撃 それ以外なら追跡
    if (dist.lengthSq() < targetDistance_ * targetDistance_) {
        return Status::SUCCESS;
    }

    // 移動
    dist = dist.normalize();
    enemyPos += dist * (speed_ * Engine::getInstance()->getDeltaTime());
    // セット
    enemy_->setTranslate(enemyPos);
    return Status::RUNNING;
}
} // namespace EnemyBehavior
