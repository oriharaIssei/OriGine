#include "CollisionCheckSystem.h"

/// stl
#include <algorithm>

/// ECS
// component
#include "component/collision/collider/Collider.h"
#include "component/collision/CollisionPushBackInfo.h"
#include "component/transform/Transform.h"
// shape
#include "component/collision/collider/AABBCollider.h"
#include "component/collision/collider/OBBCollider.h"
#include "component/collision/collider/SphereCollider.h"

// func
#include "system/collision/CollisionCheckPairFunc.h"

using namespace OriGine;

CollisionCheckSystem::CollisionCheckSystem()
    : ISystem(SystemCategory::Collision) {}

CollisionCheckSystem::~CollisionCheckSystem() {}

void CollisionCheckSystem::Initialize() {
    constexpr size_t reserveSize = 100;
    entities_.reserve(reserveSize);
}

void CollisionCheckSystem::Update() {
    EraseDeadEntity();

    entityItr_ = entities_.begin();

    // 衝突判定の記録開始処理
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

        auto collPushbackInfo = GetComponent<CollisionPushBackInfo>(entity);
        if (collPushbackInfo) {
            collPushbackInfo->ClearInfo();
        }
    }

    // エンティティごとの更新処理
    for (auto& entity : entities_) {
        UpdateEntity(entity);
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
    }
}

void CollisionCheckSystem::Finalize() {
    entities_.clear();
}

void CollisionCheckSystem::UpdateEntity(EntityHandle _handle) {
    ++entityItr_;

    Scene* currentScene = GetScene();

    auto aCollPushbackInfo       = GetComponent<CollisionPushBackInfo>(_handle);
    auto& aEntityAabbColliders   = GetComponents<AABBCollider>(_handle);
    auto& aEntitySphereColliders = GetComponents<SphereCollider>(_handle);
    auto& aEntityObbColliders    = GetComponents<OBBCollider>(_handle);

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
                if (CheckCollisionPair<>(currentScene, aEntity, bEntity, colliderA.GetWorldShape(), colliderB.GetWorldShape(), _aInfo, _bInfo)) {
                    colliderA.SetCollisionState(bEntity);
                    colliderB.SetCollisionState(aEntity);
                }
            }
        }
    };

    for (auto bItr = entityItr_; bItr != entities_.end(); ++bItr) {
        EntityHandle bEntity = *bItr;

        auto bCollPushbackInfo       = GetComponent<CollisionPushBackInfo>(bEntity);
        auto& bEntityAabbColliders   = GetComponents<AABBCollider>(bEntity);
        auto& bEntitySphereColliders = GetComponents<SphereCollider>(bEntity);
        auto& bEntityObbColliders    = GetComponents<OBBCollider>(bEntity);

        if (!aEntityAabbColliders.empty()) {
            if (!bEntityAabbColliders.empty()) {
                checkCollisions(_handle, bEntity, aEntityAabbColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (!bEntitySphereColliders.empty()) {
                checkCollisions(_handle, bEntity, aEntityAabbColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (!bEntityObbColliders.empty()) {
                checkCollisions(_handle, bEntity, aEntityAabbColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
        }
        if (!aEntitySphereColliders.empty()) {
            if (!bEntityAabbColliders.empty()) {
                checkCollisions(_handle, bEntity, aEntitySphereColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (!bEntitySphereColliders.empty()) {
                checkCollisions(_handle, bEntity, aEntitySphereColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (!bEntityObbColliders.empty()) {
                checkCollisions(_handle, bEntity, aEntitySphereColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
        }
        if (!aEntityObbColliders.empty()) {
            if (!bEntityAabbColliders.empty()) {
                checkCollisions(_handle, bEntity, aEntityObbColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (!bEntitySphereColliders.empty()) {
                checkCollisions(_handle, bEntity, aEntityObbColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (!bEntityObbColliders.empty()) {
                checkCollisions(_handle, bEntity, aEntityObbColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
        }
    }
}
