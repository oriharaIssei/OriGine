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
/// <summary>
/// 規定時間 待機する 
/// </summary>
class IdleAction
    : public Action {
public:
    IdleAction(float _time);
    ~IdleAction();

    Status tick() override;

private:
    float leftTime_ = 0.0f;
};
} // namespace EnemyBehavior
