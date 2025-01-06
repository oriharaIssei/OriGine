#pragma once

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
class KnockBackAction
    : public Action {
public:
    /// <summary>
    /// ノックバックの挙動を行う
    /// </summary>
    /// <param name="knockBackSpeed">吹っ飛ぶスピード</param>
    KnockBackAction(
        const Vector3& _direction,
        float _speed);
    ~KnockBackAction();

    Status tick() override;

private:
    Vector3 velocity_ = {0.0f, 0.0f, 0.0f};
};
} // namespace EnemyBehavior
