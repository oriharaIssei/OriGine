#pragma once
#include "system/ISystem.h"

/// <summary>
/// 衝突判定後に押し戻し処理を行うシステム
/// </summary>
class CollisionPushBackSystem
    : public OriGine::ISystem {
public:
    CollisionPushBackSystem()
        : ISystem(SystemCategory::Collision) {}
    ~CollisionPushBackSystem() {}

    void Initialize() override;
    // void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(Entity* _entity) override;
};
