#include "PlayerRootBehavior.h"

//host
#include "../Player.h"
//behavior
#include "../PlayerBehaviors/PlayerDodgeBehavior.h"
#include "../PlayerBehaviors/PlayerWeakAttackBehavior.h"

///engine
#include "Engine.h"
//lib
#include "input/Input.h"

///math
#include "Quaternion.h"

PlayerRootBehavior::PlayerRootBehavior(Player* _player)
    : IPlayerBehavior(_player),
      speed_{"Game", "PlayerRoot", "speed"} {}
PlayerRootBehavior::~PlayerRootBehavior() {}
void PlayerRootBehavior::Init() {
    currentUpdate_ = [this]() {
        this->Action();
    };
}
void PlayerRootBehavior::Update() {
    currentUpdate_();

    if (input->isPadActive()) {
        if (input->isTriggerButton(XINPUT_GAMEPAD_A)) {
            Vec3f playerFront = axisZ * MakeMatrix::RotateQuaternion(player_->getRotate());
            player_->ChangeBehavior(new PlayerDodgeBehavior(player_, playerFront));
        } else if (input->isTriggerButton(XINPUT_GAMEPAD_X)) {
            player_->ChangeBehavior(new PlayerWeakAttackBehavior(player_, 0));
        }
    }
}
void PlayerRootBehavior::StartUp() {}
void PlayerRootBehavior::Action() {
    // 入力 に 応じた 方向を 取得，計算
    lastDir_ = player_->RotateUpdateByStick(0.3f);
    // ジャンプ
    if (player_->getOnGround()) {
        if (input->isTriggerButton(XINPUT_GAMEPAD_Y)) {
            player_->setJumpForce(8.0f);
        }
    }

    if (input->getLStickVelocity().lengthSq() <= 0.000000001f) {
        return;
    }

    { // 方向と速度を 使って 次の座標を計算
        // 速度を 秒単位に
        float speedPerSecond = speed_ * player_->DeltaTime();
        // 現在の 座標
        const Vec3f& playerPos = player_->getTranslate();
        player_->setTranslate(playerPos + lastDir_.normalize() * speedPerSecond * input->getLStickVelocity().length());
    }
}
void PlayerRootBehavior::EndLag() {}
