#pragma once
#include "system/ISystem.h"

namespace OriGine {

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
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
