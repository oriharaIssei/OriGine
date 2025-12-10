#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// SpriteAnimationを再生するシステム
/// </summary>
class SpriteAnimationSystem
    : public ISystem {
public:
    SpriteAnimationSystem();
    ~SpriteAnimationSystem() override;

    void Initialize() override;
    void Finalize() override;

protected:
    void UpdateEntity(Entity* _entity) override;
};

} // namespace OriGine
