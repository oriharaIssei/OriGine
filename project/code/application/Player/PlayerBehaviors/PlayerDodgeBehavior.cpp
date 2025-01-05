#include "PlayerDodgeBehavior.h"

#include "../Player.h"
#include "Engine.h"
#include "Matrix4x4.h"
#include "PlayerRootBehavior.h"
#include "Vector3.h"
#include "animation/Animation.h"
PlayerDodgeBehavior::PlayerDodgeBehavior(Player* _player, const Vector3& direction)
    : IPlayerBehavior(_player),
      actionTime_{"Game", "PlayerDodge", "actionTime"},
      endLagTime_{"Game", "PlayerDodge", "endLagTime"},
      distance_{"Game", "PlayerDodge", "distance"} {
    //beforePos の 初期化
    beforePos_ = player_->getTranslate();
    //afterPos の 初期化
    afterPos_ = beforePos_ + direction * (distance_);
}
PlayerDodgeBehavior::~PlayerDodgeBehavior() {}
void PlayerDodgeBehavior::Init() {
    // Timer の 初期化
    currentTimer_ = 0.0f;
    // Update の 初期化
    currentUpdate_ = [this]() {
        this->Action();
    };

    AnimationSetting dodgeAnimation = AnimationSetting("PlayerDodge");
    player_->getDrawObject3d()->setAnimation(dodgeAnimation.targetAnimationDirection, dodgeAnimation.name + ".anm");
    while (true) {
        if (player_->getDrawObject3d()->getAnimation()->getData()) {
            break;
        }
    }
}
void PlayerDodgeBehavior::Update() {
    currentUpdate_();
}
void PlayerDodgeBehavior::StartUp() {}
void PlayerDodgeBehavior::Action() {
    currentTimer_ += Engine::getInstance()->getDeltaTime();
    player_->setTranslate(Lerp(beforePos_, afterPos_, currentTimer_ / actionTime_));
    if (currentTimer_ >= actionTime_) {
        currentTimer_ = 0.0f;

        AnimationSetting idleAnimation = AnimationSetting("PlayerIdle");
        player_->getDrawObject3d()->setAnimation(idleAnimation.targetAnimationDirection, idleAnimation.name + ".anm");

        currentUpdate_ = [this]() {
            this->EndLag();
        };
    }
}
void PlayerDodgeBehavior::EndLag() {
    currentTimer_ += Engine::getInstance()->getDeltaTime();
    if (currentTimer_ >= endLagTime_) {
        currentTimer_ = 0.0f;
        player_->ChangeBehavior(new PlayerRootBehavior(player_));
    }
}
