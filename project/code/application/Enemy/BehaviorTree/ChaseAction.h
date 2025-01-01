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
class ChaseAction
    : public Action {
public:
    ChaseAction(float _targetDistance);
    ~ChaseAction();

    Status tick() override;

private:
    float targetDistance_ = 0.0f;
    float speed_          = 0.0f;
};
} // namespace EnemyBehavior
