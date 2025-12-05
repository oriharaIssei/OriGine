#pragma once

/// parent
#include "system/ISystem.h"

/// <summary>
/// 衝突判定システム
/// </summary>
class CollisionCheckSystem
    : public OriGine::ISystem {
public:
    CollisionCheckSystem();
    ~CollisionCheckSystem();

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(Entity* _entity) override;

protected:
    std::vector<Entity*> entities_;
    std::vector<Entity*>::iterator entityItr_;
};
