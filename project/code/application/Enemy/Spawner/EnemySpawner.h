#pragma once

/// engine
//lib
#include "globalVariables/SerializedField.h"

/// application
#include "../IEnemy.h"
#include "application/GameObject/GameObject.h"

//math
#include "Vector3.h"

class EnemyManager;
class Collider;

class EnemySpawner
    : public GameObject {
public:
    EnemySpawner(IEnemy* _enemy = nullptr, int index = 0);
    ~EnemySpawner();

    void Init();
    void Update();

    std::unique_ptr<IEnemy> Spawn();

private:
    EnemyManager* enemyManager_ = nullptr;
    IEnemy* cloneOrigine_       = nullptr;

    std::unique_ptr<Collider> hitCollider_ = nullptr;

    SerializedField<float> spawnCoolTime_;
    float leftCoolTime_ = 0.0f;

    SerializedField<float> maxHp_;
    float hp_ = 0.0f;

    SerializedField<Vector3> position_;

    bool isAlive_ = false;

public:
    void setEnemyManager(EnemyManager* enemyManager) {
        enemyManager_ = enemyManager;
    }
    void setCloneOrigine(IEnemy* cloneOrigine) {
        cloneOrigine_ = cloneOrigine;
    }

    Collider* getHitCollider() const { return hitCollider_.get(); }

    bool getIsAlive() const { return isAlive_; }
    void setIsAlive(bool isAlive) {
        isAlive_ = isAlive;
    }

    float getHp() const { return hp_; }
    void setHp(float hp) {
        hp_ = hp;
    }
    void Damage(float damage) {
        hp_ -= damage;
    }
};
