#pragma once
#include "system/ISystem.h"

class TextureEffectAnimation
    : public ISystem {
public:
    TextureEffectAnimation() : ISystem(SystemType::Effect){}
    ~TextureEffectAnimation() = default;

    void Initialize() override;
    // void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(GameEntity* _entity) override;
};
