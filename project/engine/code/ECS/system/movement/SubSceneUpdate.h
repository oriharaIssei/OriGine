#pragma once

#include "system/ISystem.h"

class SubSceneUpdate
    : public ISystem {
public:
    SubSceneUpdate() : ISystem(SystemCategory::Movement, 0) {}
    void Initialize() override {}
    void Finalize() override {}

private:
    void UpdateEntity(GameEntity* _entity) override;
};
