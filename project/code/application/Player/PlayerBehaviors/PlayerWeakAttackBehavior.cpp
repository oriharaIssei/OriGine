#include "PlayerWeakAttackBehavior.h"

#include "../Player.h"
#include "Engine.h"
#include "PlayerRootBehavior.h"
PlayerWeakAttackBehavior::PlayerWeakAttackBehavior(Player* _player, int32_t _currentCombo)
    : IPlayerBehavior(_player),
      maxCombo_{"Game", "PlayerWeakAttack", "maxCombo"},
      startUpTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "startUpTime"},
      actionTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "actionTime"},
      endLagTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "endLagTime"} {
    currentCombo_ = _currentCombo;
}
PlayerWeakAttackBehavior::~PlayerWeakAttackBehavior() {}
void PlayerWeakAttackBehavior::Init() {
    currentUpdate_ = [this]() {
        StartUp();
    };
}
void PlayerWeakAttackBehavior::Update() {
    currentUpdate_();
}
void PlayerWeakAttackBehavior::StartUp() {
    currentTimer_ += Engine::getInstance()->getDeltaTime();
    if (currentTimer_ >= startUpTime_) {
        currentTimer_  = 0.0f;
        currentUpdate_ = [this]() {
            this->Action();
        };
    }
}
void PlayerWeakAttackBehavior::Action() {
    currentTimer_ += Engine::getInstance()->getDeltaTime();

    if (input->isPadActive() && !nextBehavior_ && currentCombo_ < maxCombo_) {
        if (input->isTriggerButton(XINPUT_GAMEPAD_X)) {
            nextBehavior_.reset(new PlayerWeakAttackBehavior(player_, 0));
        }
    }

    if (currentTimer_ >= actionTime_) {
        currentTimer_  = 0.0f;
        currentUpdate_ = [this]() {
            this->EndLag();
        };
    }
}
void PlayerWeakAttackBehavior::EndLag() {
    currentTimer_ += Engine::getInstance()->getDeltaTime();

    if (input->isPadActive() && !nextBehavior_ && currentCombo_ < maxCombo_) {
        if (input->isTriggerButton(XINPUT_GAMEPAD_X)) {
            nextBehavior_.reset(new PlayerWeakAttackBehavior(player_, 0));
        }
    }
    if (currentTimer_ >= endLagTime_) {
        currentTimer_ = 0.0f;
        if (nextBehavior_) {
            player_->ChangeBehavior(nextBehavior_);
            return;
        } else {
            player_->ChangeBehavior(new PlayerRootBehavior(player_));
            return;
        }
    }
