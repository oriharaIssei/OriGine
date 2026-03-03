#pragma once

#include "system/ISystem.h"

/// ECS
// component

namespace OriGine {

/// <summary>
/// 衝突イベントをトリガーにシーン遷移の通知を行うシステム
/// </summary>
class CollisionTriggeredSceneTransition
    : public ISystem {
public:
    CollisionTriggeredSceneTransition();
    ~CollisionTriggeredSceneTransition() override = default;
    void Initialize() override;
    void Finalize() override;

private:
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
