#pragma once

/// parent
#include "system/ISystem.h"

/// stl
#include <vector>

namespace OriGine {

/// <summary>
/// 衝突判定システム
/// </summary>
class CollisionCheckSystem
    : public ISystem {
public:
    CollisionCheckSystem();
    ~CollisionCheckSystem();

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(EntityHandle _handle) override;

protected:
    ::std::vector<EntityHandle>::iterator entityItr_;
};

} // namespace OriGine
