#include "WeakEnemy.h"

#include "../Player/Player.h"
#include "engine/Engine.h"
#include "myRandom/MyRandom.h"
#include "object3d/Object3d.h"

WeakEnemy::WeakEnemy()
    : IEnemy("WeakEnemy"),
      minPlayer2Distance_("Game", "WeakEnemy", "minPlayer2Distance"),
      maxPlayer2Distance_("Game", "WeakEnemy", "maxPlayer2Distance") {
    player2Distance_ = MyRandom::Float(minPlayer2Distance_, maxAttackCoolTime_).get();
}

WeakEnemy::~WeakEnemy() {}

void WeakEnemy::Init() {
    drawObject3d_ = std::make_unique<Object3d>();
    drawObject3d_->Init("resource/Models", "Enemy.obj");
}

void WeakEnemy::Update() {
    if (currentHp_ <= 0.0f) {
        isAlive_ = false;
        return;
    }
    if (!isAlive_) {
        return;
    }
    drawObject3d_->UpdateTransform();
}

std::unique_ptr<IEnemy> WeakEnemy::Clone() {
    return std::unique_ptr<WeakEnemy>();
}
