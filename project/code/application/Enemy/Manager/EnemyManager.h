#pragma once

/// stl
//memory
#include <memory>
//container
#include <list>
#include <vector>
//etc
#include <stdint.h>

/// engine
//lib
#include "globalVariables/SerializedField.h"

/// application
//object
#include "../IEnemy.h"
#include "../Spawner/EnemySpawner.h"
class Player;

class EnemyManager {
public:
    EnemyManager();
    ~EnemyManager();

    void Init();
    void Update();
    void Draw();

private:
    Player* player_ = nullptr;

    SerializedField<int32_t> maxEnemyCount_;
    SerializedField<int32_t> spawnerCount_;

    std::vector<std::unique_ptr<EnemySpawner>> spawners_;

    std::list<std::unique_ptr<IEnemy>> enemies_;

public:
    Player* getPlayer() const { return player_; }
    void setPlayer(Player* player) {
        player_ = player;
    }
    int32_t getEnemyCount() const { return static_cast<int32_t>(enemies_.size()); }
    int32_t getMaxEnemyCount() const { return maxEnemyCount_; }

    bool isMaxEnemy() const { return getEnemyCount() >= maxEnemyCount_; }

    void addEnemy(std::unique_ptr<IEnemy> _enemy) {
        enemies_.push_back(std::move(_enemy));
    }
    void removeDeadEnemy();
    void removeDeadSpawner();
    void clear() { enemies_.clear(); }
};
