#include "Player.h"

///enigne
#include "Engine.h"
//assets
#include "animation/Animation.h"
//component
#include "../Collision/Collider.h"
#include "object3d/Object3d.h"

/// application
//component
#include "PlayerBehaviors/PlayerRootBehavior.h"
//object
#include "../Enemy/IEnemy.h"

Player::Player()
    : GameObject("Player"),
      hp_("Game", "Player", "hp"),
      maxMoveLenght_("Game", "Player", "maxMoveLenght") {
    currentHp_ = hp_;
}

Player::~Player() {}

void Player::Init() {
    // DrawObject
    drawObject3d_ = std::make_unique<AnimationObject3d>();
    drawObject3d_->Init(AnimationSetting("PlayerIdle"));

    // Behavior
    currentBehavior_ = std::make_unique<PlayerRootBehavior>(this);
    currentBehavior_->Init();

    // Collider
    hitCollider_ = std::make_unique<Collider>("Player");
    hitCollider_->Init([this](GameObject* object) {
        // null check
        if (!object) {
            return;
        }

        if (object->getID() != "EnemyAttack") {
            return;
        }

        // Damage
        if (isInvisible_) {
            return;
        }
        IEnemy* enemy = reinterpret_cast<IEnemy*>(object);
        currentHp_ -= enemy->getAttack();

        // set invisible
        isInvisible_   = true;
        invisibleTime_ = 0.7f;
    });
    hitCollider_->setHostObject(this);
    hitCollider_->setParent(&drawObject3d_->transform_);
}

void Player::Update() {
    currentBehavior_->Update();

    { // Transform Update
        if (drawObject3d_->transform_.translate.lengthSq() >= maxMoveLenght_ * maxMoveLenght_) {
            drawObject3d_->transform_.translate = drawObject3d_->transform_.translate.normalize() * maxMoveLenght_;
        }

        drawObject3d_->Update(Engine::getInstance()->getDeltaTime());
        hitCollider_->UpdateMatrix();
    }

    if (invisibleTime_ >= 0.0f) {
        invisibleTime_ -= Engine::getInstance()->getDeltaTime();
        if (invisibleTime_ < 0.0f) {
            isInvisible_ = false;
        }
    }
}

void Player::ChangeBehavior(IPlayerBehavior* next) {
    currentBehavior_.reset(next);
    currentBehavior_->Init();
}

void Player::ChangeBehavior(std::unique_ptr<IPlayerBehavior>& next) {
    currentBehavior_ = std::move(next);
    currentBehavior_->Init();
}
