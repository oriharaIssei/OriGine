#pragma once
#include "system/ISystem.h"

class SpriteAnimationSystem
    : public ISystem {
public:
    SpriteAnimationSystem();
    ~SpriteAnimationSystem() override;

    void Initialize() override;
    void Finalize() override;

protected:
    void UpdateEntity(GameEntity* _entity) override;
};
