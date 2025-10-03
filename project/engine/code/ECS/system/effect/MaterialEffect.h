#pragma once
#include "system/ISystem.h"
class MaterialEffect
    : public ISystem {
public:
    MaterialEffect();
    ~MaterialEffect() override;
    void Initialize() override;
    void Finalize() override;

private:
    void UpdateEntity(GameEntity* _entity) override;
};
