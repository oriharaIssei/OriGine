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
#include "component/collision/collider/SphereCollider.h"
#include "component/collision/collider/OBBCollider.h"

// func
#include "system/collision/CollisionCheckPairFunc.h"

CollisionCheckSystem::CollisionCheckSystem()
    : ISystem(SystemCategory::Collision) {}

CollisionCheckSystem::~CollisionCheckSystem() {}

void CollisionCheckSystem::Initialize() {
    constexpr size_t reserveSize = 100;
    entities_.reserve(reserveSize);
}

void CollisionCheckSystem::Update() {
    eraseDeadEntity();

    entities_.clear();
    for (auto id : entityIDs_) {
        entities_.emplace_back(getEntity(id));
    }

    entityItr_ = entities_.begin();

    // 衝突判定の記録開始処理
    for (auto entity : entities_) {
        Transform* transform = getComponent<Transform>(entity);
        if (transform) {
            transform->UpdateMatrix();
        }

        // AABB
        const auto& aabbColliders = getComponents<AABBCollider>(entity);
        if (aabbColliders) {
            for (auto collider = aabbColliders->begin();
                collider != aabbColliders->end();
                ++collider) {
                collider->setParent(transform);
                collider->StartCollision();
            }
        }

        // Sphere
        const auto& sphereColliders = getComponents<SphereCollider>(entity);
        if (sphereColliders) {
            for (auto collider = sphereColliders->begin();
                collider != sphereColliders->end();
                ++collider) {
                collider->setParent(transform);
                collider->StartCollision();
            }
        }

        // OBB
        const auto& obbColliders = getComponents<OBBCollider>(entity);
        if (obbColliders) {
            for (auto collider = obbColliders->begin();
                collider != obbColliders->end();
                ++collider) {
                collider->setParent(transform);
                collider->StartCollision();
            }
        }

        auto collPushbackInfo = getComponent<CollisionPushBackInfo>(entity);
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
        const auto& aabbColliders = getComponents<AABBCollider>(entity);
        if (aabbColliders == nullptr) {
            continue;
        }
        for (auto collider = aabbColliders->begin();
            collider != aabbColliders->end();
            ++collider) {
            collider->EndCollision();
        }
        // Sphere
        const auto& sphereColliders = getComponents<SphereCollider>(entity);
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
    entityIDs_.clear();
}

void CollisionCheckSystem::UpdateEntity(Entity* _entity) {
    ++entityItr_;

    auto aCollPushbackInfo       = getComponent<CollisionPushBackInfo>(_entity);
    auto* aEntityAabbColliders   = getComponents<AABBCollider>(_entity);
    auto* aEntitySphereColliders = getComponents<SphereCollider>(_entity);
    auto* aEntityObbColliders    = getComponents<OBBCollider>(_entity);

    // 2つのリスト間の衝突判定をまとめる
    auto checkCollisions = [&](
                               Entity* aEntity,
                               Entity* bEntity,
                               auto& listA,
                               auto& listB,
                               CollisionPushBackInfo* _aInfo,
                               CollisionPushBackInfo* _bInfo) {
        for (auto colliderA = listA->begin(); colliderA != listA->end(); ++colliderA) {
            if (!colliderA->isActive()) {
                continue;
            }
            for (auto colliderB = listB->begin(); colliderB != listB->end(); ++colliderB) {
                if (!colliderB->isActive()) {
                    continue;
                }
                if (CheckCollisionPair<>(aEntity, bEntity, colliderA->getWorldShape(), colliderB->getWorldShape(), _aInfo, _bInfo)) {
                    colliderA->setCollisionState(bEntity->getID());
                    colliderB->setCollisionState(aEntity->getID());
                }
            }
        }
    };

    for (auto bItr = entityItr_; bItr != entities_.end(); ++bItr) {
        Entity* bEntity = *bItr;

        auto bCollPushbackInfo      = getComponent<CollisionPushBackInfo>(bEntity);
        auto bEntityAabbColliders   = getComponents<AABBCollider>(bEntity);
        auto bEntitySphereColliders = getComponents<SphereCollider>(bEntity);
        auto bEntityObbColliders    = getComponents<OBBCollider>(bEntity);

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
