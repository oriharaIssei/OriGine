#include "CollisionCheckSystem.h"

/// stl
#include <algorithm>
#include <cfloat>

/// util
#include "util/globalVariables/GlobalVariables.h"

/// ECS
// component
#include "component/collision/collider/base/Collider.h"
#include "component/collision/CollisionPushBackInfo.h"
#include "component/transform/Transform.h"
// shape
#include "component/collision/collider/AABBCollider.h"
#include "component/collision/collider/CapsuleCollider.h"
#include "component/collision/collider/OBBCollider.h"
#include "component/collision/collider/RayCollider.h"
#include "component/collision/collider/SegmentCollider.h"
#include "component/collision/collider/SphereCollider.h"

// func
#include "system/collision/CollisionCheckPairFunc.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
CollisionCheckSystem::CollisionCheckSystem()
    : ISystem(SystemCategory::Collision) {}

/// <summary>
/// デストラクタ
/// </summary>
CollisionCheckSystem::~CollisionCheckSystem() {}

/// <summary>
/// 初期化処理
/// </summary>
void CollisionCheckSystem::Initialize() {
    constexpr size_t reserveSize = 100;
    entities_.reserve(reserveSize);

    // GlobalVariablesからCellSizeを読み込み
    GlobalVariables* gv = GlobalVariables::GetInstance();
    float cellSize      = gv->GetValue<float>("Settings", "Collision", "SpatialHashCellSize");
    if (cellSize > 0.0f) {
        spatialHash_.SetCellSize(cellSize);
    }
}

/// <summary>
/// 全体の衝突判定更新
/// </summary>
void CollisionCheckSystem::Update() {
    EraseDeadEntity();

    // SpatialHashをクリア
    spatialHash_.Clear();

    // 衝突判定の記録開始処理 + SpatialHashへの登録
    for (auto entity : entities_) {
        Transform* transform = GetComponent<Transform>(entity);
        if (transform) {
            transform->UpdateMatrix();
        }

        // AABB
        auto& aabbColliders = GetComponents<AABBCollider>(entity);
        for (auto& collider : aabbColliders) {
            collider.SetParent(transform);
            collider.StartCollision();
        }
        // Sphere
        auto& sphereColliders = GetComponents<SphereCollider>(entity);
        for (auto& collider : sphereColliders) {
            collider.SetParent(transform);
            collider.StartCollision();
        }
        // OBB
        auto& obbColliders = GetComponents<OBBCollider>(entity);
        for (auto& collider : obbColliders) {
            collider.SetParent(transform);
            collider.StartCollision();
        }
        // Capsule
        auto& capsuleColliders = GetComponents<CapsuleCollider>(entity);
        for (auto& collider : capsuleColliders) {
            collider.SetParent(transform);
            collider.StartCollision();
        }
        // Segment
        auto& segmentColliders = GetComponents<SegmentCollider>(entity);
        for (auto& collider : segmentColliders) {
            collider.SetParent(transform);
            collider.StartCollision();
        }
        // Ray
        auto& rayColliders = GetComponents<RayCollider>(entity);
        for (auto& collider : rayColliders) {
            collider.SetParent(transform);
            collider.StartCollision();
        }

        auto collPushbackInfo = GetComponent<CollisionPushBackInfo>(entity);
        if (collPushbackInfo) {
            collPushbackInfo->ClearInfo();
        }

        // エンティティの包含AABBを計算してSpatialHashに登録
        Bounds::AABB entityAABB = ComputeEntityAABB(entity);
        if (entityAABB.halfSize.lengthSq() > 0.0f) {
            spatialHash_.Insert(entity, entityAABB);
        }
    }

    // SpatialHashから衝突候補ペアを取得
    spatialHash_.GetAllPairs(collisionPairs_);

    // 衝突候補ペアのみ判定
    for (const auto& [aEntity, bEntity] : collisionPairs_) {
        CheckEntityPair(aEntity, bEntity);
    }

    // 衝突判定の記録終了処理
    for (auto entity : entities_) {
        // AABB
        auto& aabbColliders = GetComponents<AABBCollider>(entity);
        for (auto& collider : aabbColliders) {
            collider.EndCollision();
        }

        // Sphere
        auto& sphereColliders = GetComponents<SphereCollider>(entity);
        for (auto& collider : sphereColliders) {
            collider.EndCollision();
        }
        // OBB
        auto& obbColliders = GetComponents<OBBCollider>(entity);
        for (auto& collider : obbColliders) {
            collider.EndCollision();
        }
        // Capsule
        auto& capsuleColliders = GetComponents<CapsuleCollider>(entity);
        for (auto& collider : capsuleColliders) {
            collider.EndCollision();
        }
        // Segment
        auto& segmentColliders = GetComponents<SegmentCollider>(entity);
        for (auto& collider : segmentColliders) {
            collider.EndCollision();
        }
        // Ray
        auto& rayColliders = GetComponents<RayCollider>(entity);
        for (auto& collider : rayColliders) {
            collider.EndCollision();
        }
    }
}

/// <summary>
/// 終了処理
/// </summary>
void CollisionCheckSystem::Finalize() {
    entities_.clear();
}

/// <summary>
/// エンティティの包含AABBを計算
/// </summary>
Bounds::AABB CollisionCheckSystem::ComputeEntityAABB(EntityHandle _entity) {
    Bounds::AABB result;
    result.center   = Vec3f(0.0f, 0.0f, 0.0f);
    result.halfSize = Vec3f(0.0f, 0.0f, 0.0f);

    bool hasCollider = false;
    Vec3f minPoint   = Vec3f(FLT_MAX, FLT_MAX, FLT_MAX);
    Vec3f maxPoint   = Vec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    auto mergeAABB = [&](const Bounds::AABB& aabb) {
        if (aabb.halfSize.lengthSq() <= 0.0f) {
            return;
        }
        hasCollider   = true;
        Vec3f aabbMin = aabb.center - aabb.halfSize;
        Vec3f aabbMax = aabb.center + aabb.halfSize;
        minPoint[X]   = std::min(minPoint[X], aabbMin[X]);
        minPoint[Y]   = std::min(minPoint[Y], aabbMin[Y]);
        minPoint[Z]   = std::min(minPoint[Z], aabbMin[Z]);
        maxPoint[X]   = std::max(maxPoint[X], aabbMax[X]);
        maxPoint[Y]   = std::max(maxPoint[Y], aabbMax[Y]);
        maxPoint[Z]   = std::max(maxPoint[Z], aabbMax[Z]);
    };

    // AABB
    auto& aabbColliders = GetComponents<AABBCollider>(_entity);
    for (auto& collider : aabbColliders) {
        if (collider.IsActive()) {
            mergeAABB(collider.ToWorldAABB());
        }
    }

    // Sphere
    auto& sphereColliders = GetComponents<SphereCollider>(_entity);
    for (auto& collider : sphereColliders) {
        if (collider.IsActive()) {
            mergeAABB(collider.ToWorldAABB());
        }
    }

    // OBB
    auto& obbColliders = GetComponents<OBBCollider>(_entity);
    for (auto& collider : obbColliders) {
        if (collider.IsActive()) {
            mergeAABB(collider.ToWorldAABB());
        }
    }

    // Capsule
    auto& capsuleColliders = GetComponents<CapsuleCollider>(_entity);
    for (auto& collider : capsuleColliders) {
        if (collider.IsActive()) {
            mergeAABB(collider.ToWorldAABB());
        }
    }

    // Segment
    auto& segmentColliders = GetComponents<SegmentCollider>(_entity);
    for (auto& collider : segmentColliders) {
        if (collider.IsActive()) {
            mergeAABB(collider.ToWorldAABB());
        }
    }

    // Ray
    auto& rayColliders = GetComponents<RayCollider>(_entity);
    for (auto& collider : rayColliders) {
        if (collider.IsActive()) {
            mergeAABB(collider.ToWorldAABB());
        }
    }

    if (hasCollider) {
        result.center   = (minPoint + maxPoint) * 0.5f;
        result.halfSize = (maxPoint - minPoint) * 0.5f;
    }

    return result;
}

/// <summary>
/// エンティティペア間の衝突判定を行う
/// </summary>
void CollisionCheckSystem::CheckEntityPair(EntityHandle _aEntity, EntityHandle _bEntity) {
    Scene* currentScene = GetScene();

    auto aCollPushbackInfo        = GetComponent<CollisionPushBackInfo>(_aEntity);
    auto& aEntityAabbColliders    = GetComponents<AABBCollider>(_aEntity);
    auto& aEntitySphereColliders  = GetComponents<SphereCollider>(_aEntity);
    auto& aEntityObbColliders     = GetComponents<OBBCollider>(_aEntity);
    auto& aEntityCapsuleColliders = GetComponents<CapsuleCollider>(_aEntity);
    auto& aEntitySegmentColliders = GetComponents<SegmentCollider>(_aEntity);
    auto& aEntityRayColliders     = GetComponents<RayCollider>(_aEntity);

    auto bCollPushbackInfo        = GetComponent<CollisionPushBackInfo>(_bEntity);
    auto& bEntityAabbColliders    = GetComponents<AABBCollider>(_bEntity);
    auto& bEntitySphereColliders  = GetComponents<SphereCollider>(_bEntity);
    auto& bEntityObbColliders     = GetComponents<OBBCollider>(_bEntity);
    auto& bEntityCapsuleColliders = GetComponents<CapsuleCollider>(_bEntity);
    auto& bEntitySegmentColliders = GetComponents<SegmentCollider>(_bEntity);
    auto& bEntityRayColliders     = GetComponents<RayCollider>(_bEntity);

    // 2つのリスト間の衝突判定をまとめる
    auto checkCollisions = [&](
                               EntityHandle aEntity,
                               EntityHandle bEntity,
                               auto& listA,
                               auto& listB,
                               CollisionPushBackInfo* _aInfo,
                               CollisionPushBackInfo* _bInfo) {
        for (auto& colliderA : listA) {
            if (!colliderA.IsActive()) {
                continue;
            }
            for (auto& colliderB : listB) {
                if (!colliderB.IsActive()) {
                    continue;
                }
                if (!colliderA.CanCollideWith(colliderB)) {
                    continue;
                }
                if (CheckCollisionPair<>(currentScene, aEntity, bEntity, colliderA.GetWorldShape(), colliderB.GetWorldShape(), _aInfo, _bInfo)) {
                    colliderA.SetCollisionState(bEntity);
                    colliderB.SetCollisionState(aEntity);
                }
            }
        }
    };

    if (!aEntityAabbColliders.empty()) {
        if (!bEntityAabbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityAabbColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySphereColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityAabbColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityObbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityAabbColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityCapsuleColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityAabbColliders, bEntityCapsuleColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySegmentColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityAabbColliders, bEntitySegmentColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityRayColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityAabbColliders, bEntityRayColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
    }
    if (!aEntitySphereColliders.empty()) {
        if (!bEntityAabbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySphereColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySphereColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySphereColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityObbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySphereColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityCapsuleColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySphereColliders, bEntityCapsuleColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySegmentColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySphereColliders, bEntitySegmentColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityRayColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySphereColliders, bEntityRayColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
    }
    if (!aEntityObbColliders.empty()) {
        if (!bEntityAabbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityObbColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySphereColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityObbColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityObbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityObbColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityCapsuleColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityObbColliders, bEntityCapsuleColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySegmentColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityObbColliders, bEntitySegmentColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityRayColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityObbColliders, bEntityRayColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
    }
    // Capsule vs All
    if (!aEntityCapsuleColliders.empty()) {
        if (!bEntityAabbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityCapsuleColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySphereColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityCapsuleColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityObbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityCapsuleColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityCapsuleColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityCapsuleColliders, bEntityCapsuleColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySegmentColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityCapsuleColliders, bEntitySegmentColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityRayColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityCapsuleColliders, bEntityRayColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
    }
    // Segment vs All
    if (!aEntitySegmentColliders.empty()) {
        if (!bEntityAabbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySegmentColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySphereColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySegmentColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityObbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySegmentColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityCapsuleColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySegmentColliders, bEntityCapsuleColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySegmentColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySegmentColliders, bEntitySegmentColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityRayColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntitySegmentColliders, bEntityRayColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
    }
    // Ray vs All
    if (!aEntityRayColliders.empty()) {
        if (!bEntityAabbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityRayColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySphereColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityRayColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityObbColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityRayColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityCapsuleColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityRayColliders, bEntityCapsuleColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntitySegmentColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityRayColliders, bEntitySegmentColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
        if (!bEntityRayColliders.empty()) {
            checkCollisions(_aEntity, _bEntity, aEntityRayColliders, bEntityRayColliders, aCollPushbackInfo, bCollPushbackInfo);
        }
    }
}
