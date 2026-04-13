#pragma once

#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// オブジェクトの伸縮を更新するシステム
/// </summary>
class ScaleDeformSystem
    : public ISystem {
public:
    ScaleDeformSystem();
    ~ScaleDeformSystem() override = default;
    void Initialize() override;
    void Finalize() override;

private:
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
