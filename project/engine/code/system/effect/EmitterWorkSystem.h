#pragma once

#include "system/ISystem.h"

class EmitterWorkSystem
    : public ISystem {
public:
    EmitterWorkSystem() : ISystem(SystemType::Effect) {}
    ~EmitterWorkSystem() {}

    void Initialize() override;
    /*virtual void Update();*/
    void Finalize() override;

protected:
    void UpdateEntity(GameEntity* _entity) override;
};
