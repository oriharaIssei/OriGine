#pragma once

//parent
#include "DefaultNodes.h"

///math
#include "math/Vector3.h"

namespace EnemyBehavior {
/// <summary>
/// ノックバック (現在のBehaviorを上書きして処理する)
/// </summary>
class KnockBackAction
    : public Action {
public:
    /// <summary>
    ///
    /// </summary>
    /// <param name="_direction"></param>
    /// <param name="_speed"></param>
    KnockBackAction(
        const Vector3& _direction,
        float _speed,
        std::unique_ptr<Node> _originalBehavior);
    ~KnockBackAction();

    Status tick() override;

private:
    Vector3 velocity_ = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Node> originalBehavior_;
};

class KnockBack
    : public Sequence {
public:
    KnockBack(
        IEnemy* _enemy,
        const Vector3& _direction,
        float _speed,
        std::unique_ptr<Node> _afterNode) {
        setEnemy(_enemy);
        auto nockBack = std::make_unique<KnockBackAction>(_direction, _speed, std::move(_afterNode));
        nockBack->setEnemy(_enemy);
        addChild(std::move(nockBack));
    }

    ~KnockBack() {}
};
} // namespace EnemyBehavior
