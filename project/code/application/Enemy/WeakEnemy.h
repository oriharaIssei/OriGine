#pragma once

///parent
#include "IEnemy.h"

//lib
#include "globalVariables/SerializedField.h"

class WeakEnemyBehavior
    : public EnemyBehavior::Sequence {
    friend class WeakEnemy;

public:
    WeakEnemyBehavior(IEnemy* _enemy);
    ~WeakEnemyBehavior();
};

class WeakEnemy
    : public IEnemy {
public:
    WeakEnemy();
    ~WeakEnemy();

    void Init() override;
    void Update() override;

    std::unique_ptr<IEnemy> Clone() override;

private:
};
