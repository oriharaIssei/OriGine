#pragma once
#include "system/ISystem.h"

class PrimitiveNodeAnimationWorkSystem
    : public ISystem {
public:
    PrimitiveNodeAnimationWorkSystem() : ISystem(SystemType::Effect) {}
    ~PrimitiveNodeAnimationWorkSystem() override = default;
    void Initialize() override {}
    // void update() override;
    void Finalize() override {}

protected:
    void UpdateEntity(GameEntity* _entity) override;
};
