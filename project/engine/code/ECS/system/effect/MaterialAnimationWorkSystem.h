#pragma once
#include "system/ISystem.h"

class MaterialAnimationWorkSystem
    : public ISystem {
public:
    MaterialAnimationWorkSystem() : ISystem(SystemCategory::Effect) {}
    ~MaterialAnimationWorkSystem() override = default;
    void Initialize() override {}
    // void update() override;
    void Finalize() override {}

protected:
    void UpdateEntity(GameEntity* _entity) override;
};
