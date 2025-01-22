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
      hitStopTime_{"Game", "PlayerWeakAttack" + std::to_string(_currentCombo), "hitStopTime"} {
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
    currentTimer_ += Engine::getInstance()->getGameDeltaTime();
    if (currentTimer_ >= startUpTime_) {
        currentTimer_ = 0.0f;

        CreateAttackCollider();
        currentUpdate_ = [this]() {
            this->Action();
        };
    }
}

void PlayerWeakAttackBehavior::Action() {
    currentTimer_ += Engine::getInstance()->getGameDeltaTime();

    CheckCombo();

    if (currentTimer_ >= actionTime_) {
        // attackColliderを消す
        player_->getAttackCollider()->getHitCollider()->setIsAlive(false);
        // Playerの座標をアニメーション終了後の座標に変更
        {
            auto playerModel    = player_->getDrawObject3d()->getModel();
            int32_t bodyIndex   = playerModel->meshData_->meshIndexes["body"];
            auto& bodyMesh      = playerModel->meshData_->mesh_[bodyIndex];
            auto& bodyTransform = playerModel->transformBuff_[&bodyMesh].openData_;
            player_->setTranslate(bodyTransform.worldMat[3]);
        }

        // アニメーションを変更
        AnimationSetting weakAttackActionSettings = AnimationSetting("PlayerIdle");
        player_->getDrawObject3d()->setNextAnimation(weakAttackActionSettings.targetAnimationDirection, weakAttackActionSettings.name + ".anm", startUpTime_);

        currentTimer_ = 0.0f;
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

                Engine::getInstance()->getGameDeltaTimeInstance()->HitStop(hitStopScale_, hitStopTime_);
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

                Engine::getInstance()->getGameDeltaTimeInstance()->HitStop(hitStopScale_, hitStopTime_);
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
