#include "benchmark/BenchmarkSceneBuilder.h"

/// stl
#include <random>

/// ECS
#include "component/collision/collider/SphereCollider.h"
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"
#include "scene/Scene.h"
#include "system/ISystem.h"

/// math
#include "Vector3.h"

/// externals
#include "logger/Logger.h"

using namespace OriGine;

namespace OriGine::Benchmark {

namespace {

constexpr int32_t kMovementPriority  = 0; // Movementカテゴリ内の優先順位(ベンチでは1システムのみなので0で十分)
constexpr int32_t kCollisionPriority = 0; // Collisionカテゴリ内の優先順位

// ベンチ用エンティティの初期速度の最大値[unit/s]。
// 大きすぎるとextentで意図した密度が数フレームで崩れてしまうため、小さく抑える。
constexpr float kMaxInitialSpeed = 2.0f;

} // namespace

void BuildBenchmarkScene(Scene* _scene, const BenchmarkConfig& _config) {
    if (!_scene) {
        LOG_ERROR("BuildBenchmarkScene: scene is null.");
        return;
    }

    // CollisionCheckSystem::CheckEntityPair の負荷を再現するために必要な2システムを登録する
    _scene->RegisterSystem("MoveSystemByRigidBody", kMovementPriority, true);
    _scene->RegisterSystem("CollisionCheckSystem", kCollisionPriority, true);

    auto moveSystem      = _scene->GetSystem("MoveSystemByRigidBody");
    auto collisionSystem = _scene->GetSystem("CollisionCheckSystem");
    if (!moveSystem || !collisionSystem) {
        LOG_ERROR("BuildBenchmarkScene: failed to register required systems (MoveSystemByRigidBody / CollisionCheckSystem).");
        return;
    }

    // 決定性を担保するため std::mt19937 を固定シードで使用する。
    // std::random_device や時刻を混ぜると実行のたびにシーンが変わり、前後比較ベンチとして意味を失うため絶対に行わない。
    std::mt19937 rng(_config.seed);
    const float halfExtent = _config.extent * 0.5f;
    std::uniform_real_distribution<float> posDist(-halfExtent, halfExtent);
    std::uniform_real_distribution<float> velDist(-kMaxInitialSpeed, kMaxInitialSpeed);

    for (uint32_t i = 0; i < _config.entityCount; ++i) {
        EntityHandle handle = _scene->CreateEntity("BenchmarkEntity", false);

        _scene->AddComponent<Transform>(handle);
        _scene->AddComponent<Rigidbody>(handle);
        _scene->AddComponent<SphereCollider>(handle);

        Transform* transform = _scene->GetComponent<Transform>(handle);
        if (transform) {
            transform->translate = Vec3f(posDist(rng), posDist(rng), posDist(rng));
            transform->UpdateMatrix();
        }

        Rigidbody* rigidbody = _scene->GetComponent<Rigidbody>(handle);
        if (rigidbody) {
            rigidbody->SetVelocity(Vec3f(velDist(rng), velDist(rng), velDist(rng)));
        }

        SphereCollider* sphere = _scene->GetComponent<SphereCollider>(handle);
        if (sphere) {
            sphere->SetLocalRadius(_config.radius);
        }

        // CollisionCheckSystem / MoveSystemByRigidBody の走査対象として登録する
        moveSystem->AddEntity(handle);
        collisionSystem->AddEntity(handle);
    }

    LOG_INFO("BuildBenchmarkScene: generated {} entities (extent={}, radius={}, seed={}).",
        _config.entityCount, _config.extent, _config.radius, _config.seed);
}

} // namespace OriGine::Benchmark
