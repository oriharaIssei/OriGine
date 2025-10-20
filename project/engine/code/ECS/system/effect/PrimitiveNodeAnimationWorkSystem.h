#pragma once
#include "system/ISystem.h"

/// <summary>
/// PrimitiveNodeAnimationのアニメーション更新を行うシステム
/// </summary>
class PrimitiveNodeAnimationWorkSystem
    : public ISystem {
public:
    PrimitiveNodeAnimationWorkSystem() : ISystem(SystemCategory::Effect) {}
    ~PrimitiveNodeAnimationWorkSystem() override = default;
    void Initialize() override {}
    void Finalize() override {}

protected:
    void UpdateEntity(Entity* _entity) override;
};
