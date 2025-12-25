#pragma once

#include "system/ISystem.h"

/// ECS
// component
#include "component/animation/DissolveAnimation.h"
#include <entity/EntityHandle.h>

namespace OriGine {

/// <summary>
/// DissolveAnimation を更新するシステム
/// </summary>
class DissolveAnimationSystem
    : public ISystem {
public:
    DissolveAnimationSystem();
    ~DissolveAnimationSystem() override;
    void Initialize() override;
    void Finalize() override;
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
