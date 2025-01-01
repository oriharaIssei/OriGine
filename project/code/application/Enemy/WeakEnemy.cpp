#include "WeakEnemy.h"

#include "../Player/Player.h"
#include "engine/Engine.h"
#include "myRandom/MyRandom.h"
#include "object3d/Object3d.h"

WeakEnemy::WeakEnemy()
    : IEnemy("WeakEnemy"),
      minPlayer2Distance_("Game", "WeakEnemy", "minPlayer2Distance"),
      maxPlayer2Distance_("Game", "WeakEnemy", "maxPlayer2Distance") {
    currentPlayer2Distance_ = MyRandom::Float(minPlayer2Distance_, maxAttackCoolTime_).get();
}

WeakEnemy::~WeakEnemy() {}

void WeakEnemy::Init() {
    drawObject3d_ = std::make_unique<Object3d>();
    drawObject3d_->Init("resource/Models", "Enemy.obj");
}

void WeakEnemy::Update() {
    { // follow player
        Vector3 playerPos = player_->getTranslate();
        Vector3 enemyPos  = transform_.translate;
        Vector3 dir       = playerPos - enemyPos;

        // 規定距離内なら 攻撃 それ以外なら追跡
        if (dir.lengthSq() < currentPlayer2Distance_ * currentPlayer2Distance_) {
            currentAttackCoolTime_ -= Engine::getInstance()->getDeltaTime();
            // 攻撃
            if (currentAttackCoolTime_ < 0.0f) {
                // player_->Damage(currentAttack_);
                // currentAttackCoolTime_ = MyRandom::Float(minAttackCoolTime_, maxAttackCoolTime_).get();
            }
        } else {
            dir.normalize();
            transform_.translate += dir * speed_;
        }
    }

    drawObject3d_->Update();
}

std::unique_ptr<IEnemy> WeakEnemy::Clone() {
    return std::unique_ptr<WeakEnemy>();
}
