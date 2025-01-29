#include "WeakAttackAction.h"

//parent
#include "../IEnemy.h"
//assets
#include "animation/Animation.h"
//object
#include "../../AttackCollider/AttackCollider.h"
#include "../../Player/Player.h"

namespace EnemyBehavior {
CreateAttackCollider::CreateAttackCollider(
    const std::string& _colliderID,
    const Vec3f& _colliderOffset,
    std::function<void(GameObject*)> onCollision)
    : colliderID_(_colliderID),
      colliderOffset_(_colliderOffset),
      onCollision_(onCollision) {}

CreateAttackCollider::~CreateAttackCollider() {}

Status CreateAttackCollider::tick() {
    AttackCollider* collider = enemy_->getAttackCollider();
    collider->Init();

    Vec3f offset = TransformVector(colliderOffset_, MakeMatrix::RotateQuaternion(enemy_->getRotate()));

    collider->ColliderInit(enemy_->getTranslate() + offset, onCollision_);
    return Status::SUCCESS;
}

WeakAttackAction::WeakAttackAction(float _attack)
    : attackPower_(_attack) {}

WeakAttackAction::~WeakAttackAction() {}

Status WeakAttackAction::tick() {
    // 攻撃 アニメーションを流すだけ
    isEndMotion_ = enemy_->getDrawObject3d()->getAnimation()->isEnd();
    if (!isEndMotion_) {
        return Status::RUNNING;
    }
    if (enemy_->getAttackCollider()) {
        enemy_->resetAttackCollider();
    }
    return Status::SUCCESS;
}
} // namespace EnemyBehavior
