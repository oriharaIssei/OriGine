#pragma once
#include "system/ISystem.h"

class ParticleUpdateSystem
    : public ISystem {
public:
    ParticleUpdateSystem()
        : ISystem(SystemType::Movement) {}
    void Initialize() override;
    // void Update() override;
    void Finalize() override;

private:
    void UpdateEntity(GameEntity* _entity) override;
};
