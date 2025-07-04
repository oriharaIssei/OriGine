#pragma once
#include "system/ISystem.h"

class SkinningAnimationSystem
    : public ISystem {
public:
    SkinningAnimationSystem();
    ~SkinningAnimationSystem() override;

    void Initialize();
    void Finalize();

protected:
    void UpdateEntity(GameEntity* _entity)override;

};
