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

    entities_.clear();
    for (auto id : entities_) {
        entities_.emplace_back(GetEntity(id));
    }

    entityItr_ = entities_.begin();

    // 衝突判定の記録開始処理
    for (auto entity : entities_) {
        Transform* transform = GetComponent<Transform>(entity);
        if (transform) {
            transform->UpdateMatrix();
        }

        // AABB
        const auto& aabbColliders = GetComponents<AABBCollider>(entity);
        if (aabbColliders) {
            for (auto collider = aabbColliders->begin();
                collider != aabbColliders->end();
                ++collider) {
                collider->SetParent(transform);
                collider->StartCollision();
            }
        }

        // Sphere
        const auto& sphereColliders = GetComponents<SphereCollider>(entity);
        if (sphereColliders) {
            for (auto collider = sphereColliders->begin();
                collider != sphereColliders->end();
                ++collider) {
                collider->SetParent(transform);
                collider->StartCollision();
            }
        }

        // OBB
        const auto& obbColliders = GetComponents<OBBCollider>(entity);
        if (obbColliders) {
            for (auto collider = obbColliders->begin();
                collider != obbColliders->end();
                ++collider) {
                collider->SetParent(transform);
                collider->StartCollision();
            }
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
        const auto& aabbColliders = GetComponents<AABBCollider>(entity);
        if (aabbColliders == nullptr) {
            continue;
        }
        for (auto collider = aabbColliders->begin();
            collider != aabbColliders->end();
            ++collider) {
            collider->EndCollision();
        }
        // Sphere
        const auto& sphereColliders = GetComponents<SphereCollider>(entity);
        if (sphereColliders == nullptr) {
            continue;
        }
        for (auto collider = sphereColliders->begin();
            collider != sphereColliders->end();
            ++collider) {
            collider->EndCollision();
        }
    }
}

void CollisionCheckSystem::Finalize() {
    entities_.clear();
}

void CollisionCheckSystem::UpdateEntity(EntityHandle _handle) {
    ++entityItr_;

    Scene* currentScene = GetScene();

    auto aCollPushbackInfo      = GetComponent<CollisionPushBackInfo>(_entity);
    auto aEntityAabbColliders   = GetComponents<AABBCollider>(_entity);
    auto aEntitySphereColliders = GetComponents<SphereCollider>(_entity);
    auto aEntityObbColliders    = GetComponents<OBBCollider>(_entity);

    // 2つのリスト間の衝突判定をまとめる
    auto checkCollisions = [&](
                               Entity* aEntity,
                               Entity* bEntity,
                               auto& listA,
                               auto& listB,
                               CollisionPushBackInfo* _aInfo,
                               CollisionPushBackInfo* _bInfo) {
        for (auto colliderA = listA->begin(); colliderA != listA->end(); ++colliderA) {
            if (!colliderA->IsActive()) {
                continue;
            }
            for (auto colliderB = listB->begin(); colliderB != listB->end(); ++colliderB) {
                if (!colliderB->IsActive()) {
                    continue;
                }
                if (CheckCollisionPair<>(currentScene, aEntity, bEntity, colliderA->GetWorldShape(), colliderB->GetWorldShape(), _aInfo, _bInfo)) {
                    colliderA->SetCollisionState(bEntity->GetID());
                    colliderB->SetCollisionState(aEntity->GetID());
                }
            }
        }
    };

    for (auto bItr = entityItr_; bItr != entities_.end(); ++bItr) {
        Entity* bEntity = *bItr;

        auto bCollPushbackInfo      = GetComponent<CollisionPushBackInfo>(bEntity);
        auto bEntityAabbColliders   = GetComponents<AABBCollider>(bEntity);
        auto bEntitySphereColliders = GetComponents<SphereCollider>(bEntity);
        auto bEntityObbColliders    = GetComponents<OBBCollider>(bEntity);

        if (aEntityAabbColliders) {
            if (bEntityAabbColliders) {
                checkCollisions(_entity, bEntity, aEntityAabbColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (bEntitySphereColliders) {
                checkCollisions(_entity, bEntity, aEntityAabbColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (bEntityObbColliders) {
                checkCollisions(_entity, bEntity, aEntityAabbColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
        }
        if (aEntitySphereColliders) {
            if (bEntityAabbColliders) {
                checkCollisions(_entity, bEntity, aEntitySphereColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (bEntitySphereColliders) {
                checkCollisions(_entity, bEntity, aEntitySphereColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (bEntityObbColliders) {
                checkCollisions(_entity, bEntity, aEntitySphereColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
        }
        if (aEntityObbColliders) {
            if (bEntityAabbColliders) {
                checkCollisions(_entity, bEntity, aEntityObbColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (bEntitySphereColliders) {
                checkCollisions(_entity, bEntity, aEntityObbColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (bEntityObbColliders) {
                checkCollisions(_entity, bEntity, aEntityObbColliders, bEntityObbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
        }
    }
}
