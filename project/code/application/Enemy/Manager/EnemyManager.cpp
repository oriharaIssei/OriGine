#include "EnemyManager.h"

#include "../WeakEnemy.h"
#include "application/Player/Player.h"

EnemyManager::EnemyManager()
    : maxEnemyCount_("Game", "EnemyManager", "maxEnemyCount"),
      spawnerCount_("Game", "EnemyManager", "spawnerCount") {}

EnemyManager::~EnemyManager() {
}

void EnemyManager::Init() {
    for (int32_t i = 0; i < spawnerCount_; ++i) {
        auto& spawner = spawners_.emplace_back(new EnemySpawner(new WeakEnemy(), i));
        spawner->Init();
        spawner->setEnemyManager(this);
    }
}

void EnemyManager::Update() {
    removeDeadSpawner();
    removeDeadEnemy();

    for (auto& spawner : spawners_) {
        spawner->Update();
    }

    for (auto& enemy : enemies_) {
        enemy->Update();
    }
}

void EnemyManager::Draw() {
    for (auto& spawner : spawners_) {
        spawner->Draw();
    }
    for (auto& enemy : enemies_) {
        enemy->Draw();
    }
}

void EnemyManager::removeDeadEnemy() {
    std::erase_if(
        enemies_,
        [](const std::unique_ptr<IEnemy>& enemy) {
            return !enemy->getIsAlive() || enemy == nullptr;
        });
}

void EnemyManager::removeDeadSpawner() {
    std::erase_if(
        spawners_,
        [](const std::unique_ptr<EnemySpawner>& spawner) {
            return !spawner->getIsAlive() || spawner == nullptr;
        });
}