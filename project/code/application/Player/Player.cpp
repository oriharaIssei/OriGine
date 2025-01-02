#include "Player.h"

#include "Engine.h"
#include "PlayerBehaviors/PlayerRootBehavior.h"
#include "object3d/Object3d.h"
#include "transform/Transform.h"

Player::Player()
    : hp_("Game", "Player", "hp") {
    currentHp_ = hp_;
}

Player::~Player() {}

void Player::Init() {
    // 座標系
    transform_.UpdateMatrix();

    // DrawObject
    drawObject3d_                    = std::make_unique<Object3d>();
    drawObject3d_->Init("resource/Models", "Enemy.obj");
    drawObject3d_->transform_.parent = &transform_;

    // Behavior
    currentBehavior_ = std::make_unique<PlayerRootBehavior>(this);
    currentBehavior_->Init();
}

void Player::Update() {
    currentBehavior_->Update();

    { // Transform Update
        transform_.UpdateMatrix();
        drawObject3d_->UpdateTransform();
    }
}

void Player::Draw() {
    drawObject3d_->Draw();
}

void Player::ChangeBehavior(IPlayerBehavior* next) {
    currentBehavior_.reset(next);
    currentBehavior_->Init();
}
