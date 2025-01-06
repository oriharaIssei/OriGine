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
class IsNearToPlayer
    : public Condition {
public:
    IsNearToPlayer(float _distance);
    ~IsNearToPlayer();
    Status tick() override;
};
class ChaseAction
    : public Action {
public:
    ChaseAction(float _speed, float _targetDistance);
    ~ChaseAction();

    Status tick() override;

private:
    float speed_          = 0.0f;
    float targetDistance_ = 0.0f;
};
} // namespace EnemyBehavior
