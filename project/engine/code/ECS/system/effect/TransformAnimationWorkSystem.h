#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// トランスフォーム アニメーションワークシステム
/// </summary>
class TransformAnimationWorkSystem
    : public ISystem {
public:
    TransformAnimationWorkSystem() : ISystem(SystemCategory::Effect) {}
    ~TransformAnimationWorkSystem() override = default;
    void Initialize() override {}
    // void update() override;
    void Finalize() override {}

protected:
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
