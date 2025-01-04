#include "EnemySpawner.h"

///engine
#include "Engine.h"
//assetes
#include "animation/Animation.h"
///applicatoin
#include "../Manager/EnemyManager.h"
#include "application/Collision/Collider.h"

EnemySpawner::EnemySpawner(IEnemy* _enemy, int index)
    : cloneOrigine_(_enemy),
      spawnCoolTime_("Game", "EnemySpawner", "spawnCoolTime"),
      maxHp_("Game", "EnemySpawner", "maxHp"),
      position_("Game", "EnemySpawner" + std::to_string(index), "position") {}

EnemySpawner::~EnemySpawner() {
}

void EnemySpawner::Init() {
    isAlive_ = true;

    leftCoolTime_ = spawnCoolTime_;
    hp_           = maxHp_;

    // DrawObject
    drawObject3d_ = std::make_unique<AnimationObject3d>();
    drawObject3d_->Init(AnimationSetting("EnemySpawner"));
    drawObject3d_->transform_.translate = position_;

    // Collider
    hitCollider_ = std::make_unique<Collider>("EnemySpawner");
    hitCollider_->Init();
    hitCollider_->setHostObject(this);
    hitCollider_->setParent(&drawObject3d_->transform_);
}

void EnemySpawner::Update() {
    if (hp_ <= 0.0f) {
        isAlive_ = false;
        return;
    }
    if (!isAlive_) {
        return;
    }

    leftCoolTime_ -= Engine::getInstance()->getDeltaTime();
    if (leftCoolTime_ <= 0.0f && !enemyManager_->isMaxEnemy()) {
        leftCoolTime_ = spawnCoolTime_;
        // Spawn
        enemyManager_->addEnemy(std::move(Spawn()));
    }

    setTranslate(position_);
    drawObject3d_->Update(Engine::getInstance()->getDeltaTime());
}

std::unique_ptr<IEnemy> EnemySpawner::Spawn() {
    std::unique_ptr<IEnemy> clone = std::move(cloneOrigine_->Clone());
    clone->Init();
    clone->setPlayer(enemyManager_->getPlayer());
    return clone;
}
