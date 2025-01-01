#include "ChaseAction.h"

///engine
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
    { // follow player
        Vector3 playerPos = enemy_->getPlayer()->getTranslate();
        Vector3 enemyPos  = enemy_->getTranslate();
        Vector3 dist       = playerPos - enemyPos;

        // 規定距離内なら 攻撃 それ以外なら追跡
        if (dist.lengthSq() < _targetDistance * _targetDistance) {
            return Status::SUCCESS;
        }

        dist = dist.normalize();
        enemyPos += dist * speed_;
    }
}
} // namespace EnemyBehavior
