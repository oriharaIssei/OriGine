#include "PlayerWeakAttackBehavior.h"
///parent
#include "../Player.h"
//behavior
#include "PlayerRootBehavior.h"

///Engine
#include "Engine.h"
#include "animation/Animation.h"
///application
// object
#include "application/Enemy/IEnemy.h"
///Collider
#include "application/AttackCollider/AttackCollider.h"

PlayerWeakAttackBehavior::PlayerWeakAttackBehavior(Player* _player, int32_t _currentCombo)
    : IPlayerBehavior(_player),
      maxCombo_{"Game", "PlayerWeakAttack", "maxCombo"},
      startUpTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "startUpTime"},
      actionTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "actionTime"},
      endLagTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "endLagTime"},
      attackPower_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "attackPower"},
      attackColliderOffset_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "attackColliderOffset_"} {
    currentCombo_ = _currentCombo;

    AnimationSetting weakAttackActionSettings = AnimationSetting("Player_WeakAttack" + std::to_string(currentCombo_));
    player_->getDrawObject3d()->setNextAnimation(weakAttackActionSettings.targetAnimationDirection, weakAttackActionSettings.name + ".anm", startUpTime_);
    while (true) {
        if (player_->getDrawObject3d()->getAnimation()->getData()) {
            break;
        }
    }
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
        currentTimer_ = 0.0f;

        AttackCollider* attackCollider_ = player_->getAttackCollider();
        attackCollider_->resetRadius("PlayerWeakAttack" + std::to_string(currentCombo_));

        Vector3 colliderPos = player_->getTranslate();
        colliderPos += TransformVector(attackColliderOffset_, MakeMatrix::RotateQuaternion(player_->getRotate()));
        attackCollider_->Init();
        attackCollider_->ColliderInit(
            colliderPos,
            [this](GameObject* object) {
                if (!object) {
                    return;
                }
                if (object->getID() != "Enemy") {
                    return;
                }
                IEnemy* enemy = dynamic_cast<IEnemy*>(object);

                if (!enemy && enemy->getIsInvisible()) {
                    return;
                }

                enemy->Damage(player_->getPower() * attackPower_);
                Vector3 knockBackDirection = enemy->getTranslate() - player_->getTranslate();
                enemy->KnockBack(knockBackDirection.normalize(), 11.1f);
                enemy->setInvisibleTime(0.1f);
            });
        currentUpdate_ = [this]() {
            this->Action();
        };
    }
}

void PlayerWeakAttackBehavior::Action() {
    currentTimer_ += Engine::getInstance()->getDeltaTime();

    if (input->isPadActive() && !nextBehavior_ && currentCombo_ + 1 < maxCombo_) {
        if (input->isTriggerButton(XINPUT_GAMEPAD_X)) {
            nextBehavior_.reset(new PlayerWeakAttackBehavior(player_, currentCombo_ + 1));
        }
    }

    if (currentTimer_ >= actionTime_) {
        currentTimer_ = 0.0f;
        player_->getAttackCollider()->getHitCollider()->setIsAlive(false);
        currentUpdate_ = [this]() {
            this->EndLag();
        };

        AnimationSetting weakAttackActionSettings = AnimationSetting("PlayerIdle");
        player_->getDrawObject3d()->setNextAnimation(weakAttackActionSettings.targetAnimationDirection, weakAttackActionSettings.name + ".anm", startUpTime_);
    }
}

void PlayerWeakAttackBehavior::EndLag() {
    currentTimer_ += Engine::getInstance()->getDeltaTime();

    if (nextBehavior_) {
        player_->ChangeBehavior(nextBehavior_);
        return;
    } else {
        if (input->isPadActive() && currentCombo_ + 1 < maxCombo_) {
            if (input->isTriggerButton(XINPUT_GAMEPAD_X)) {
                nextBehavior_.reset(new PlayerWeakAttackBehavior(player_, currentCombo_ + 1));
            }
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
}
