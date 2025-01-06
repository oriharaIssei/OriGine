#include "ChaseAction.h"

///engine
#include "Engine.h"
//component
#include "../IEnemy.h"
#include "application/Player/Player.h"
//lib
#include "myRandom/MyRandom.h"

namespace EnemyBehavior {
IsNearToPlayer::IsNearToPlayer(float _distance)
    : distance_(_distance) {}
IsNearToPlayer::~IsNearToPlayer() {
}
Status IsNearToPlayer::tick() {
    Vector3 playerPos = enemy_->getPlayer()->getTranslate();
    Vector3 enemyPos  = enemy_->getTranslate();
    Vector3 dist      = playerPos - enemyPos;
    if (dist.lengthSq() < distance_ * distance_) {
        return Status::SUCCESS;
    }
    return Status::FAILURE;
}

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
    dist                            = dist.normalize();
    const Quaternion& currentRotate = enemy_->getRotate();
    { // Player を 入力方向 へ 回転
        Quaternion inputDirectionRotate = Quaternion::RotateAxisAngle({0.0f, 1.0f, 0.0f}, atan2(dist.x, dist.z));
        inputDirectionRotate            = inputDirectionRotate.normalize();
        enemy_->setRotate(LerpShortAngle(currentRotate, inputDirectionRotate, 0.3f).normalize());

        if (std::isnan(currentRotate.x)) {
            enemy_->setRotate(inputDirectionRotate);
        }
    }
    enemyPos += dist * (speed_ * Engine::getInstance()->getDeltaTime());
    // セット
    enemy_->setTranslate(enemyPos);
    return Status::RUNNING;
}
} // namespace EnemyBehavior
