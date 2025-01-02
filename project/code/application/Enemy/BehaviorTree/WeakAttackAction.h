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
class WeakAttackAction
    : public Action {
public:
    WeakAttackAction(float _attack);
    ~WeakAttackAction();

    Status tick() override;

private:
    float attack_ = 0.0f;
    bool isEndMotion_ = false;
};
} // namespace EnemyBehavior
