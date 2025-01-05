#pragma once

//parent
#include "DefaultNodes.h"

///stl
//string
#include <string>

///engine
//lib
#include "globalVariables/SerializedField.h"

namespace EnemyBehavior {
class CreateAttackCollider
    : public Action {
public:
    CreateAttackCollider(
        const std::string& _colliderID,
        const Vector3& _colliderOffset);
    ~CreateAttackCollider();

    Status tick() override;

private:
    std::string colliderID_;
    Vector3 colliderOffset_;
};

class WeakAttackAction
    : public Action {
public:
    WeakAttackAction(float _attack);
    ~WeakAttackAction();

    Status tick() override;

private:
    float attackPower_ = 0.0f;
    bool isEndMotion_  = false;
};
} // namespace EnemyBehavior
