#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// PointLight / SpotLight の位置・方向を、同一エンティティの Transform から同期するシステム.
/// SystemCategory::Effect で動作し、Render より前に実行される.
/// </summary>
class LightTransformSyncSystem
    : public ISystem {
public:
    LightTransformSyncSystem() : ISystem(SystemCategory::Effect) {}
    ~LightTransformSyncSystem() override = default;

    void Initialize() override {}
    void Finalize() override {}

protected:
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
