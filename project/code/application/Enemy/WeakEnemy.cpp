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
#include "../Player/Player.h"

#pragma region WeakEnemyBehavior
#include "BehaviorTree/ChaseAction.h"
#include "BehaviorTree/DefaultNodes.h"
#include "BehaviorTree/IdleAction.h"
#include "BehaviorTree/WeakAttackAction.h"

class WeakEnemyBehavior
    : public EnemyBehavior::Sequence {
    friend class WeakEnemy;

public:
    WeakEnemyBehavior(WeakEnemy* _enemy) {
        setEnemy(_enemy);
        auto chase = std::make_unique<EnemyBehavior::ChaseAction>(_enemy->getSpeed(), _enemy->getPlayer2Distance());
        chase->setEnemy(_enemy);
        auto idle = std::make_unique<EnemyBehavior::IdleAction>(3.0f);
        idle->setEnemy(_enemy);
        auto secondChase = std::make_unique<EnemyBehavior::ChaseAction>(_enemy->getSpeed(), 2.0f);
        secondChase->setEnemy(_enemy);
        auto attack = std::make_unique<EnemyBehavior::WeakAttackAction>(enemy_->getAttack());
        attack->setEnemy(_enemy);

        addChild(std::move(chase));
        addChild(std::move(idle));
        addChild(std::move(secondChase));
        addChild(std::move(attack));
    }
    ~WeakEnemyBehavior() {}
};

#pragma endregion

WeakEnemy::WeakEnemy()
    : IEnemy("WeakEnemy"),
      minPlayer2Distance_("Game", "WeakEnemy", "minPlayer2Distance"),
      maxPlayer2Distance_("Game", "WeakEnemy", "maxPlayer2Distance") {
    player2Distance_ = MyRandom::Float(minPlayer2Distance_, maxPlayer2Distance_).get();
}

WeakEnemy::~WeakEnemy() {}

void WeakEnemy::Init() {
    isAlive_ = true;

    // DrawObject
    drawObject3d_ = std::make_unique<AnimationObject3d>();
    drawObject3d_->Init(AnimationSetting("EnemyIdle"));

    // Collider
    hitCollider_ = std::make_unique<Collider>("WeakEnemy");
    hitCollider_->Init();
    hitCollider_->setHostObject(this);
    hitCollider_->setParent(&drawObject3d_->transform_);

    behaviorTree_ = std::make_unique<WeakEnemyBehavior>(this);
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

    drawObject3d_->Update(Engine::getInstance()->getDeltaTime());
}

std::unique_ptr<IEnemy> WeakEnemy::Clone() {
    return std::make_unique<WeakEnemy>();
}
