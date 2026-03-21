#include "EntitySpawnerWorkSystem.h"

/// engine
#include "Engine.h"
#include "scene/SceneFactory.h"

/// ECS
#include "component/spawner/EntitySpawner.h"
#include "component/transform/Transform.h"
#include "ECS/HandleAssignMode.h"

using namespace OriGine;

void EntitySpawnerWorkSystem::Initialize() {}

void EntitySpawnerWorkSystem::Finalize() {
    entities_.clear();
}

void EntitySpawnerWorkSystem::UpdateEntity(EntityHandle _handle) {
    const float deltaTime = Engine::GetInstance()->GetDeltaTime();

    auto& spawners = GetComponents<EntitySpawner>(_handle);
    if (spawners.empty()) {
        return;
    }

    SceneFactory factory;

    for (auto& spawner : spawners) {
        if (!spawner.IsActive()) {
            continue;
        }

        if (spawner.templateTypeName_.empty()) {
            continue;
        }

        // スポーン数を取得（タイミング管理は Emitter に委譲）
        int32_t spawnCount = spawner.emitter_.Update(deltaTime);

        // ワールド原点を更新（ループ前に1回だけ。pre/current の差が補間に使われる）
        spawner.emitter_.UpdateWorldOriginPos();

        for (int32_t i = 0; i < spawnCount; ++i) {
            Entity* spawned = factory.BuildEntityFromTemplate(
                GetScene(),
                spawner.templateTypeName_,
                HandleAssignMode::GenerateNew);

            if (!spawned) {
                continue;
            }

            // スポーン位置をシェイプから決定し Transform に反映
            // （Transform コンポーネントが存在する場合のみ）
            auto* transform = GetComponent<Transform>(spawned->GetHandle());
            if (transform) {
                transform->translate = spawner.emitter_.GetInterpolatedOriginPos(i, spawnCount) + spawner.emitter_.GetSpawnPos();
            }
        }

        // 時間切れなら非アクティブ化
        if (spawner.emitter_.IsExpired()) {
            spawner.emitter_.Deactivate();
        }
    }
}
