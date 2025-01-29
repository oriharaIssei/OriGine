#include "PlayerWeakAttackBehavior.h"
///parent
#include "../Player.h"
//behavior
#include "PlayerRootBehavior.h"
#include "application/HitEffectManager/HitEffectManager.h"
///Engine
#include "Engine.h"
#include "animation/Animation.h"
// object
#include "application/Enemy/IEnemy.h"
#include "application/Enemy/Spawner/EnemySpawner.h"
///Collider
#include "application/AttackCollider/AttackCollider.h"
//lib
#include "myRandom/MyRandom.h"

PlayerWeakAttackBehavior::PlayerWeakAttackBehavior(Player* _player, int32_t _currentCombo)
    : IPlayerBehavior(_player),
      maxCombo_{"Game", "PlayerWeakAttack", "maxCombo"},
      startUpTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "startUpTime"},
      actionTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "actionTime"},
      endLagTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "endLagTime"},
      attackPower_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "attackPower"},
      attackColliderOffset_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "attackColliderOffset_"},
      knockBackPower_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "knockBackPower"},
      hitStopScale_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "hitStopScale"},
      hitStopTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "hitStopTime"},
      hitDetectionTimeForAttack_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "hitDetectionTimeForAttack"},
      rotateInterpolateInStartup_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "rotateInterpolateInStartup"},
      rotateInterpolateInAttack_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "rotateInterpolateInAttack"},
      velocityInStartup_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "velocityInStartup"},
      velocityInAttack_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "velocityInAttack"} {
    currentCombo_ = _currentCombo;

    AnimationSetting weakAttackActionSettings = AnimationSetting("Player_WeakAttack" + std::to_string(currentCombo_));
    player_->getDrawObject3d()->setAnimation(weakAttackActionSettings.targetAnimationDirection, weakAttackActionSettings.name + ".anm");

    while (true) {
        if (player_->getDrawObject3d()->getAnimation()->getData()) {
            break;
        }
    }

    speedInStartup_     = velocityInStartup_->length();
    directionInStartup_ = velocityInStartup_->normalize();
    speedInAttack_      = velocityInAttack_->length();
    directionInAttack_  = velocityInAttack_->normalize();
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
    currentTimer_ += player_->DeltaTime();

    if (rotateInterpolateInStartup_ != 0.f) {
        lastDirection_ = player_->RotateUpdateByStick(rotateInterpolateInStartup_);
    }
    if (speedInStartup_ != 0.f) {
        player_->setTranslate(player_->getTranslate() + lastDirection_ * (speedInStartup_ * Engine::getInstance()->getDeltaTime()));
    }

    if (currentTimer_ >= startUpTime_) {
        currentTimer_ = 0.0f;

         AnimationSetting weakAttackActionSettings = AnimationSetting("Player_WeakAttack" + std::to_string(currentCombo_));
        player_->getDrawObject3d()->setAnimation(weakAttackActionSettings.targetAnimationDirection, weakAttackActionSettings.name + ".anm");

        CreateAttackCollider();
        currentUpdate_ = [this]() {
            this->Action();
        };
    }
}

void PlayerWeakAttackBehavior::Action() {
    currentTimer_ += player_->DeltaTime();

    CheckCombo();

    if (rotateInterpolateInAttack_ != 0.f) {
        lastDirection_ = player_->RotateUpdateByStick(rotateInterpolateInAttack_);
    }
    if (speedInAttack_ != 0.f) {
        player_->setTranslate(player_->getTranslate() + lastDirection_ * (speedInAttack_ * Engine::getInstance()->getDeltaTime()));
    }

    if (currentTimer_ >= actionTime_) {
        // attackColliderを消す
        player_->getAttackCollider()->getHitCollider()->setIsAlive(false);

        // アニメーションを変更
        AnimationSetting weakAttackActionSettings = AnimationSetting("PlayerIdle");
        player_->getDrawObject3d()->setAnimation(weakAttackActionSettings.targetAnimationDirection, weakAttackActionSettings.name + ".anm");

        currentTimer_  = 0.0f;
        currentUpdate_ = [this]() {
            this->EndLag();
        };
    }
}

void PlayerWeakAttackBehavior::EndLag() {
    currentTimer_ += Engine::getInstance()->getDeltaTime();

    CheckCombo();
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

void PlayerWeakAttackBehavior::CreateAttackCollider() {
    AttackCollider* attackCollider_ = player_->getAttackCollider();
    attackCollider_->resetRadius("PlayerWeakAttack" + std::to_string(currentCombo_));

    Vec3f colliderPos = player_->getTranslate();
    colliderPos += TransformVector(attackColliderOffset_, MakeMatrix::RotateQuaternion(player_->getRotate()));
    attackCollider_->Init();
    attackCollider_->ColliderInit(
        colliderPos,
        [this](GameObject* object) {
            if (!object) {
                return;
            }
            Quaternion effectRotate;
            Vec3f effectPos;
            if (object->getID() == "Player") {
                return;
            } else if (object->getID() == "Enemy") {
                IEnemy* enemy = dynamic_cast<IEnemy*>(object);

                if (!enemy || enemy->getIsInvisible()) {
                    return;
                }

                effectPos = enemy->getTranslate();

                enemy->Damage(player_->getPower() * attackPower_);
                Vec3f knockBackDirection = enemy->getTranslate() - player_->getTranslate();
                knockBackDirection[Y]    = 0.0f;
                enemy->KnockBack(knockBackDirection.normalize(), knockBackPower_);
                enemy->setInvisibleTime(actionTime_ - currentTimer_);

                Vec2f directionForEffect = Vec2f(player_->getTranslate()[X], player_->getTranslate()[Z]) - Vec2f(effectPos[X], effectPos[Z]).normalize();
                effectRotate             = Quaternion::RotateAxisAngle({0.0f, 1.0f, 0.0f}, atan2(directionForEffect[X], directionForEffect[Y]));

                HitEffectManager* hitEffectManager = HitEffectManager::getInstance();
                hitEffectManager->addHitEffect(effectRotate, effectPos);

                enemy->HitStop(hitStopScale_, hitStopTime_);
            } else if (object->getID() == "EnemySpawner") {
                EnemySpawner* enemySpawner = dynamic_cast<EnemySpawner*>(object);

                if (!enemySpawner || enemySpawner->getIsInvisible()) {
                    return;
                }

                effectPos = object->getTranslate();

                enemySpawner->Damage(player_->getPower() * attackPower_);
                enemySpawner->setInvisibleTime(actionTime_ - currentTimer_);

                Vec2f directionForEffect = Vec2f(player_->getTranslate()[X], player_->getTranslate()[Z]) - Vec2f(effectPos[X], effectPos[Z]).normalize();
                effectRotate             = Quaternion::RotateAxisAngle({0.0f, 1.0f, 0.0f}, atan2(directionForEffect[X], directionForEffect[Y]));

                HitEffectManager* hitEffectManager = HitEffectManager::getInstance();
                hitEffectManager->addHitEffect(effectRotate, effectPos);

                enemySpawner->HitStop(hitStopScale_, hitStopTime_);
            }
        });
}

void PlayerWeakAttackBehavior::CheckCombo() {
    if (nextBehavior_) {
        return;
    }
    if (input->isPadActive() && !nextBehavior_ && currentCombo_ + 1 < maxCombo_) {
        if (input->isTriggerButton(XINPUT_GAMEPAD_X)) {
            nextBehavior_.reset(new PlayerWeakAttackBehavior(player_, currentCombo_ + 1));
        }
    }
}
