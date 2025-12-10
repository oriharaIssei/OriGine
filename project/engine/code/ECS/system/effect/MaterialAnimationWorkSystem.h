#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// マテリアルアニメーションワークシステム
/// </summary>
class MaterialAnimationWorkSystem
    : public ISystem {
public:
    MaterialAnimationWorkSystem() : ISystem(SystemCategory::Effect) {}
    ~MaterialAnimationWorkSystem() override = default;
    void Initialize() override {}
    // void update() override;
    void Finalize() override {}

protected:
    void UpdateEntity(Entity* _entity) override;
};

} // namespace OriGine
