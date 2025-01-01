#include "IEnemy.h"

#include "myRandom/MyRandom.h"
#include "object3d/Object3d.h"

IEnemy::IEnemy(const std::string& _type)
    : hp_("Game", _type, "hp"),
      speed_("Game", _type, "speed"),
      attack_("Game", _type, "attack"),
      maxAttackCoolTime_("Game", _type, "maxAttackCoolTime"),
      minAttackCoolTime_("Game", _type, "minAttackCoolTime") {
    // Init Status
    currentHp_     = hp_;
    currentSpeed_  = speed_;
    currentAttack_ = attack_;
    // minAttackCoolTime_ ~ maxAttackCoolTime_
    currentAttackCoolTime_ = MyRandom::Float(minAttackCoolTime_, maxAttackCoolTime_).get();
}

IEnemy::~IEnemy() {
    // Destructor implementation
}

void IEnemy::Draw() {
    drawObject3d_->Draw();
}
