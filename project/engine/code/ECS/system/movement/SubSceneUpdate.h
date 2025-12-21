#pragma once

#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// SubSceneの更新を行うシステム
/// </summary>
class SubSceneUpdate
    : public ISystem {
public:
    SubSceneUpdate() : ISystem(SystemCategory::Movement, 0) {}
    void Initialize() override {}
    void Finalize() override {}

private:
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
