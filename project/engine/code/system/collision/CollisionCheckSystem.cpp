#include "CollisionCheckSystem.h"

/// stl
#include <algorithm>

/// ECS
#include "ECSManager.h"
// component
#include "component/collider/Collider.h"
#include "component/transform/Transform.h"

#pragma region "CheckCollisionPair"
template <>
bool CheckCollisionPair(
    const Sphere& _shapeA,
    const Sphere& _shapeB) {

    Vec3f distance = (_shapeA.center_) - (_shapeB.center_);

    if (distance.lengthSq() >= (_shapeA.radius_ + _shapeB.radius_) * (_shapeA.radius_ + _shapeB.radius_)) {
        return false;
    }
    return true;
}

template <>
bool CheckCollisionPair(
    const AABB& _shapeA,
    const AABB& _shapeB) {
    if (_shapeA.max_[X] < _shapeB.min_[X] || _shapeA.min_[X] > _shapeB.max_[X]) {
        return false;
    }
    if (_shapeA.max_[Y] < _shapeB.min_[Y] || _shapeA.min_[Y] > _shapeB.max_[Y]) {
        return false;
    }
    if (_shapeA.max_[Z] < _shapeB.min_[Z] || _shapeA.min_[Z] > _shapeB.max_[Z]) {
        return false;
    }
    return true;
}

template <>
bool CheckCollisionPair(
    const AABB& _shapeA, const Sphere& _shapeB) {
    Vec3f closest = {
        std::clamp(_shapeB.center_[X], _shapeA.min_[X], _shapeA.max_[X]),
        std::clamp(_shapeB.center_[Y], _shapeA.min_[Y], _shapeA.max_[Y]),
        std::clamp(_shapeB.center_[Z], _shapeA.min_[Z], _shapeA.max_[Z])};

    Vec3f distance = closest - _shapeB.center_;
    return distance.lengthSq() < _shapeB.radius_ * _shapeB.radius_;
};

template <>
bool CheckCollisionPair(
    // 順番が A, B と B, A で同じ処理を行うため、関数を呼び出しを入れ替える
    const Sphere& _shapeA, const AABB& _shapeB) {
    return CheckCollisionPair<AABB, Sphere>(_shapeB, _shapeA);
};

#pragma endregion

CollisionCheckSystem::CollisionCheckSystem()
    : ISystem(SystemType::Collision) {
}

CollisionCheckSystem::~CollisionCheckSystem() {}

void CollisionCheckSystem::Initialize() {
    entities_.reserve(ECSManager::getInstance()->getEntityCapacity());
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
        transform->Update();

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

void CollisionCheckSystem::UpdateEntity(GameEntity* _entity) {
    ++entityItr_;

    auto* aEntityAabbColliders   = getComponents<AABBCollider>(_entity);
    auto* aEntitySphereColliders = getComponents<SphereCollider>(_entity);

    // 2つのリスト間の衝突判定をまとめる
    auto checkCollisions = [&](auto& listA, auto& listB, GameEntity* aEntity, GameEntity* bEntity) {
        for (auto colliderA = listA->begin(); colliderA != listA->end(); ++colliderA) {
            if (!colliderA->isActive()) {
                continue;
            }
            for (auto colliderB = listB->begin(); colliderB != listB->end(); ++colliderB) {
                if (!colliderB->isActive()) {
                    continue;
                }
                if (CheckCollisionPair<>(colliderA->getWorldShape(), colliderB->getWorldShape())) {
                    colliderA->setCollisionState(bEntity);
                    colliderB->setCollisionState(aEntity);
                }
            }
        }
    };

    for (auto bItr = entityItr_; bItr != entities_.end(); ++bItr) {
        GameEntity* bEntity         = *bItr;
        auto bEntityAabbColliders   = getComponents<AABBCollider>(bEntity);
        auto bEntitySphereColliders = getComponents<SphereCollider>(bEntity);

        if (aEntityAabbColliders) {
            if (bEntityAabbColliders) {
                checkCollisions(aEntityAabbColliders, bEntityAabbColliders, _entity, bEntity);
            }
            if (bEntitySphereColliders) {
                checkCollisions(aEntityAabbColliders, bEntitySphereColliders, _entity, bEntity);
            }
        }
        if (aEntitySphereColliders) {
            if (bEntityAabbColliders) {
                checkCollisions(aEntitySphereColliders, bEntityAabbColliders, _entity, bEntity);
            }
            if (bEntitySphereColliders) {
                checkCollisions(aEntitySphereColliders, bEntitySphereColliders, _entity, bEntity);
            }
        }
    }
}
