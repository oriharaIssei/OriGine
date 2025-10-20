#pragma once

#include "system/ISystem.h"

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
    void UpdateEntity(Entity* _entity) override;
};
