#pragma once

#include "system/ISystem.h"

class ScreenRenderingSystem
    : public ISystem {
public:
    ScreenRenderingSystem() : ISystem(SystemType::PostRender, 100000) {}
    ~ScreenRenderingSystem() override = default;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(GameEntity* /*_entity*/) override {}
};
