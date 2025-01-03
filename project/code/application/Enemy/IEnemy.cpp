#include "IEnemy.h"

#include "myRandom/MyRandom.h"
#include "object3d/Object3d.h"

IEnemy::IEnemy(const std::string& _type)
    : hp_("Game", _type, "hp"),
      speed_("Game", _type, "speed"),
      attack_("Game", _type, "attack") {
    // Init Status
    currentHp_     = hp_;
    currentSpeed_  = speed_;
    currentAttack_ = attack_;
}

IEnemy::~IEnemy() {
    // Destructor implementation
}

void IEnemy::Draw() {
    drawObject3d_->Draw();
}
