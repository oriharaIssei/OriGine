#include "IEnemy.h"


//behavior
#include "WeakEnemy.h"
#include "BehaviorTree/KnockBack.h"

///engine
#include "Engine.h"
//component
#include "object3d/Object3d.h"
//lib
#include "myRandom/MyRandom.h"

IEnemy::IEnemy(const std::string& _type)
    : GameObject("Enemy"),
      hp_("Game", _type, "hp"),
      speed_("Game", _type, "speed"),
      attack_("Game", _type, "attack"),
      maxMoveLenght_("Game", _type, "maxMoveLenght") {
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

    // Shadow
    {
        shadowObject_->transform_.translate = (Vec3f(drawObject3d_->transform_.translate[X], -0.03f, drawObject3d_->transform_.translate[Z]));
        shadowObject_->UpdateTransform();
    }
    Object3d::setBlendMode(BlendMode::Sub);
    shadowObject_->Draw();
    Object3d::setBlendMode(BlendMode::Alpha);
}

void IEnemy::KnockBack(const Vec3f& direction, float speed) {
    behaviorTree_ = std::make_unique<EnemyBehavior::KnockBack>(this, direction, speed, std::make_unique<WeakEnemyBehavior>(this));
}

void IEnemy::setInvisibleTime(float time) {
    isInvisible_   = true;
    invisibleTime_ = time;

    for (auto& material :
         drawObject3d_->getModel()->materialData_) {
        material.material = Engine::getInstance()->getMaterialManager()->Create("Player_Inbisible");
    }
}
