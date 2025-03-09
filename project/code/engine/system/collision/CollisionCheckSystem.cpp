#include "CollisionCheckSystem.h"

/// stl
#include <algorithm>

/// ECS
#include "ECSManager.h"
// component
#include "collider/Collider.h"
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

void CollisionCheckSystem::Init() {
}

void CollisionCheckSystem::Update() {
    entityItr_ = entities_.begin();

    for (auto& entity : entities_) {
        std::vector<Collider>* colliders = getComponents<Collider>(entity);
        if (!colliders) {
            continue;
        }

        for (auto& collider : *colliders) {
            collider.StartCollision();
        }
    }

    for (auto& entity : entities_) {
        UpdateEntity(entity);
    }

    for (auto& entity : entities_) {
        std::vector<Collider>* colliders = getComponents<Collider>(entity);
        if (!colliders) {
            continue;
        }

        for (auto& collider : *colliders) {
            collider.EndCollision();
        }
    }
}

void CollisionCheckSystem::Finalize() {
    entities_.clear();
}

void CollisionCheckSystem::UpdateEntity(GameEntity* _entity) {
    ++entityItr_;

    std::vector<Collider>* aColliders = getComponents<Collider>(_entity);

    // collider　が ない場合は処理を行わない
    if (!aColliders || aColliders->empty()) {
        return;
    }

    for (std::vector<GameEntity*>::iterator itr = entityItr_;
        itr != entities_.end();
        ++itr) {
        for (auto& aCollider : *aColliders) {
            if (!aCollider.isActive()) {
                continue;
            }

            std::vector<Collider>* bColliders = getComponents<Collider>(*itr);
            if (!bColliders) {
                continue;
            }

            // aCollider の形状情報をキャッシュ
            IShape* shapeA           = aCollider.getWorldShape();
            CollisionShapeType typeA = aCollider.getShapeType();

            for (auto& bCollider : *bColliders) {
                if (!bCollider.isActive()) {
                    continue;
                }

                // bCollider の形状情報
                IShape* shapeB           = bCollider.getWorldShape();
                CollisionShapeType typeB = bCollider.getShapeType();

                bool collisionDetected = false;
                // ToDo : cast による Shape の変換
                if (typeA == CollisionShapeType::Sphere && typeB == CollisionShapeType::Sphere) {
                    const Sphere* sphereA = static_cast<const Sphere*>(shapeA);
                    const Sphere* sphereB = static_cast<const Sphere*>(shapeB);
                    collisionDetected     = CheckCollisionPair<Sphere, Sphere>(*sphereA, *sphereB);
                } else if (typeA == CollisionShapeType::AABB && typeB == CollisionShapeType::AABB) {
                    const AABB* aabbA = static_cast<const AABB*>(shapeA);
                    const AABB* aabbB = static_cast<const AABB*>(shapeB);
                    collisionDetected = CheckCollisionPair<AABB, AABB>(*aabbA, *aabbB);
                } else if (typeA == CollisionShapeType::AABB && typeB == CollisionShapeType::Sphere) {
                    const AABB* aabbA     = static_cast<const AABB*>(shapeA);
                    const Sphere* sphereB = static_cast<const Sphere*>(shapeB);
                    collisionDetected     = CheckCollisionPair<AABB, Sphere>(*aabbA, *sphereB);
                } else if (typeA == CollisionShapeType::Sphere && typeB == CollisionShapeType::AABB) {
                    const Sphere* sphereA = static_cast<const Sphere*>(shapeA);
                    const AABB* aabbB     = static_cast<const AABB*>(shapeB);
                    collisionDetected     = CheckCollisionPair<Sphere, AABB>(*sphereA, *aabbB);
                }

                if (collisionDetected) {
                    aCollider.setCollisionState(*itr);
                    bCollider.setCollisionState(_entity);
                }
            }
        }
    }
}
