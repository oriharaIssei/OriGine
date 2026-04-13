#pragma once

#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// EntitySpawner コンポーネントを管理するシステム。
/// スポーンタイミングが来たら SceneFactory 経由で .ent テンプレートから Entity を生成する。
/// </summary>
class EntitySpawnerWorkSystem
    : public ISystem {
public:
    EntitySpawnerWorkSystem() : ISystem(SystemCategory::Effect) {}
    ~EntitySpawnerWorkSystem() override = default;

    void Initialize() override;
    void Finalize() override;

protected:
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
