#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// 衝突判定後に押し戻し処理を行うシステム
/// </summary>
class CollisionPushBackSystem
    : public ISystem {
public:
    CollisionPushBackSystem()
        : ISystem(SystemCategory::Collision) {}
    ~CollisionPushBackSystem() {}

    void Initialize() override;
    // void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
