#include "EnemySpawner.h"

///engine
#include "Engine.h"
///applicatoin
#include "../Manager/EnemyManager.h"

EnemySpawner::EnemySpawner(IEnemy* _enemy, int index)
    : spawnCoolTime_("Game", "EnemySpawner" + std::to_string(index), "spawnCoolTime"),
      maxHp_("Game", "EnemySpawner" + std::to_string(index), "maxHp") {}

EnemySpawner::~EnemySpawner() {
}

void EnemySpawner::Init() {
    isAlive_ = true;

    leftCoolTime_ = spawnCoolTime_;
    hp_           = maxHp_;
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
    if (leftCoolTime_ <= 0.0f) {
        leftCoolTime_ = spawnCoolTime_;
        // Spawn
        enemyManager_->addEnemy(std::move(Spawn()));
    }
}
std::unique_ptr<IEnemy> EnemySpawner::Spawn() {
    std::unique_ptr<IEnemy> clone = cloneOrigine_->Clone();
    clone->Init();
    clone->setPlayer(enemyManager_->getPlayer());
    return clone;
}
