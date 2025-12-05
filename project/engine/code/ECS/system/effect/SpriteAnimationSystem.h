#pragma once
#include "system/ISystem.h"

/// <summary>
/// SpriteAnimationを再生するシステム
/// </summary>
class SpriteAnimationSystem
    : public OriGine::ISystem {
public:
    SpriteAnimationSystem();
    ~SpriteAnimationSystem() override;

    void Initialize() override;
    void Finalize() override;

protected:
    void UpdateEntity(Entity* _entity) override;
};
