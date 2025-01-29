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
        const Vec3f& _direction,
        float _speed);
    ~KnockBackAction();

    Status tick() override;

private:
    Vec3f velocity_ = {0.0f, 0.0f, 0.0f};
};

class KnockBack
    : public Sequence {
public:
    KnockBack(
        IEnemy* _enemy,
        const Vec3f& _direction,
        float _speed) {
        setEnemy(_enemy);
        auto nockBack = std::make_unique<KnockBackAction>(_direction, _speed);
        nockBack->setEnemy(_enemy);

        auto changeAnimation = std::make_unique<ChangeAnimation>("EnemyKnockBack.anm");
        changeAnimation->setEnemy(_enemy);
        addChild(std::move(changeAnimation));
        addChild(std::move(nockBack));
    }

    ~KnockBack() {}
};
} // namespace EnemyBehavior
