#include "WeakEnemy.h"

///engine
#include "engine/Engine.h"
//assets
#include "animation/Animation.h"
//component
#include "object3d/Object3d.h"
//lib
#include "myRandom/MyRandom.h"
///application
#include "../AttackCollider/AttackCollider.h"
#include "../Player/Player.h"

#pragma region WeakEnemyBehavior
#include "BehaviorTree/ChaseAction.h"
#include "BehaviorTree/DefaultNodes.h"
#include "BehaviorTree/IdleAction.h"
#include "BehaviorTree/WeakAttackAction.h"

#pragma endregion

WeakEnemy::WeakEnemy()
    : IEnemy("WeakEnemy") {}

WeakEnemy::~WeakEnemy() {}

void WeakEnemy::Init() {
    isAlive_ = true;

    // DrawObject
    drawObject3d_ = std::make_unique<AnimationObject3d>();
    drawObject3d_->Init(AnimationSetting("EnemyChase"));

    // Collider
    hitCollider_ = std::make_unique<Collider>("WeakEnemy");
    hitCollider_->Init();
    hitCollider_->setHostObject(this);
    hitCollider_->setParent(&drawObject3d_->transform_);

    behaviorTree_ = std::make_unique<WeakEnemyBehavior>(this);

    attackCollider_ = std::make_unique<AttackCollider>("EnemyAttack");
    attackCollider_->Init();
    attackCollider_->setIsAlive(false);

    // Shadow
    shadowObject_ = std::make_unique<Object3d>();
    shadowObject_->Init("resource/Models", "ShadowPlane.obj");
    {
        auto model = shadowObject_->getModel();
        while (true) {
            if (model->meshData_->currentState_ == LoadState::Loaded) {
                break;
            }
        }
        for (auto& material : model->materialData_) {
            material.material = Engine::getInstance()->getMaterialManager()->Create("Shadow");
        }
    }
    shadowObject_->transform_.scale = Vector3(2.3f, 2.3f, 2.3f);
}

void WeakEnemy::Update() {
    if (currentHp_ <= 0.0f) {
        isAlive_ = false;
        return;
    }
    if (!isAlive_) {
        return;
    }

    behaviorTree_->tick();

    { // Transform Update
        if (drawObject3d_->transform_.translate.lengthSq() >= maxMoveLenght_ * maxMoveLenght_) {
            drawObject3d_->transform_.translate = drawObject3d_->transform_.translate.normalize() * maxMoveLenght_;
        }
        drawObject3d_->Update(Engine::getInstance()->getDeltaTime());
    }

    if (invisibleTime_ > 0.0f) {
        invisibleTime_ -= Engine::getInstance()->getDeltaTime();
        if (invisibleTime_ < 0.0f) {
            isInvisible_ = false;
            for (auto& material :
                 drawObject3d_->getModel()->materialData_) {
                material.material = Engine::getInstance()->getMaterialManager()->getMaterial("white");
            }
        }
    } else {
        isInvisible_ = false;
    }
}

std::unique_ptr<IEnemy> WeakEnemy::Clone() {
    return std::make_unique<WeakEnemy>();
}

WeakEnemyBehavior::WeakEnemyBehavior(IEnemy* _enemy) {
    setEnemy(_enemy);

    auto chase = std::make_unique<EnemyBehavior::ChaseAction>(_enemy->getSpeed(), 8.0f);
    chase->setEnemy(_enemy);
    auto idle = std::make_unique<EnemyBehavior::IdleAction>(3.0f);
    idle->setEnemy(_enemy);
    auto secondChase = std::make_unique<EnemyBehavior::ChaseAction>(_enemy->getSpeed(), 2.0f);
    secondChase->setEnemy(_enemy);
    auto attack = std::make_unique<EnemyBehavior::WeakAttackAction>(enemy_->getAttack());
    attack->setEnemy(_enemy);

    auto chaseAnimation = std::make_unique<EnemyBehavior::ChangeAnimation>("EnemyChase.anm");
    chaseAnimation->setEnemy(_enemy);
    auto idleAnimation = std::make_unique<EnemyBehavior::ChangeAnimation>("EnemyIdle.anm");
    idleAnimation->setEnemy(_enemy);
    idleAnimation->LerpNextAnimation(0.1f);
    auto chaseForIdle = std::make_unique<EnemyBehavior::ChangeAnimation>("EnemyChase.anm");
    chaseForIdle->setEnemy(_enemy);
    chaseForIdle->LerpNextAnimation(0.1f);
    auto idleForAttack = std::make_unique<EnemyBehavior::ChangeAnimation>("WeakEnemy_WeakAttack.anm");
    idleForAttack->setEnemy(_enemy);
    idleForAttack->LerpNextAnimation(0.06f);

    auto createAttackCollider =
        std::make_unique<EnemyBehavior::CreateAttackCollider>(
            "EnemyAttack",
            Vector3(0.0f, 0.0f, 1.3f),
            [this](GameObject* object) {
                if (!object) {
                    return;
                }
                if (object->getID() != "Player") {
                    return;
                }
                Player* player = dynamic_cast<Player*>(object);

                if (!player || player->getInvisibleTime()) {
                    return;
                }
                player->Damage(enemy_->getAttack());
                player->setInvisibleTime(0.6f);
            });
    createAttackCollider->setEnemy(_enemy);

    addChild(std::move(chaseAnimation));
    addChild(std::move(chase));
    addChild(std::move(idleAnimation));
    addChild(std::move(idle));
    addChild(std::move(chaseForIdle));
    addChild(std::move(secondChase));
    addChild(std::move(idleForAttack));
    addChild(std::move(createAttackCollider));
    addChild(std::move(attack));
}

WeakEnemyBehavior::~WeakEnemyBehavior() {
}
