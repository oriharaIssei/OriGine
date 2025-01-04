#include "Player.h"

///enigne
#include "Engine.h"
//component
#include "../Collision/Collider.h"
#include "object3d/Object3d.h"

/// application
//component
#include "PlayerBehaviors/PlayerRootBehavior.h"

Player::Player()
    : hp_("Game", "Player", "hp") {
    currentHp_ = hp_;
}

Player::~Player() {}

void Player::Init() {
    // DrawObject
    drawObject3d_ = std::make_unique<Object3d>();
    drawObject3d_->Init("resource/Models", "Player.obj");

    // Behavior
    currentBehavior_ = std::make_unique<PlayerRootBehavior>(this);
    currentBehavior_->Init();

    // Collider
    hitCollider_ = std::make_unique<Collider>("Player");
    hitCollider_->Init();
    hitCollider_->setHostObject(this);
    hitCollider_->setParent(&drawObject3d_->transform_);
}

void Player::Update() {
    currentBehavior_->Update();

    { // Transform Update
        drawObject3d_->UpdateTransform();
        hitCollider_->UpdateMatrix();
    }
}

void Player::ChangeBehavior(IPlayerBehavior* next) {
    currentBehavior_.reset(next);
    currentBehavior_->Init();
}
