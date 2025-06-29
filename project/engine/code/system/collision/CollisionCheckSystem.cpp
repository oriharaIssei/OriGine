#include "CollisionCheckSystem.h"

/// stl
#include <algorithm>

/// ECS
#include "ECSManager.h"
// component
#include "component/collider/Collider.h"
#include "component/collider/CollisionPushBackInfo.h"
#include "component/transform/Transform.h"

#pragma region "CheckCollisionPair"

template <typename ShapeA, typename ShapeB>
bool CheckCollisionPair(
    GameEntity* _entityA,
    GameEntity* _entityB,
    [[maybe_unused]] const ShapeA& _shapeA,
    [[maybe_unused]] const ShapeB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {
    return false;
}

template <>
bool CheckCollisionPair(
    GameEntity* _entityA,
    GameEntity* _entityB,
    const Sphere& _shapeA,
    const Sphere& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {

    Vec3f distance = (_shapeA.center_) - (_shapeB.center_);

    if (distance.lengthSq() >= (_shapeA.radius_ + _shapeB.radius_) * (_shapeA.radius_ + _shapeB.radius_)) {
        return false;
    }

    bool aIsPushBack = _aInfo && _aInfo->getPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _bInfo && _bInfo->getPushBackType() != CollisionPushBackType::None;

    if (!aIsPushBack && !bIsPushBack) {
        return true; // 衝突情報がない場合は何もしない
    }

    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collNormal      = distance.normalize();
    float overlapDistance = (_shapeA.radius_ + _shapeB.radius_) - distance.length();

    if (aIsPushBack) {
        // 衝突時の処理
        CollisionPushBackInfo::Info info;
        info.collVec   = collNormal * overlapDistance * overlapRate;
        info.collPoint = _shapeA.center_ + info.collVec.normalize() * _shapeA.radius_;
        _aInfo->AddCollisionInfo(_entityB->getID(), info);
    }
    if (bIsPushBack) {
        // 衝突時の処理
        CollisionPushBackInfo::Info info;
        info.collVec   = -collNormal * overlapDistance * overlapRate;
        info.collPoint = _shapeB.center_ + info.collVec.normalize() * _shapeB.radius_;
        _bInfo->AddCollisionInfo(_entityA->getID(), info);
    }

    return true;
}

template <>
bool CheckCollisionPair(
    GameEntity* _entityA,
    GameEntity* _entityB,
    const AABB& _shapeA,
    const AABB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {

    if (_shapeA.max_[X] < _shapeB.min_[X] || _shapeA.min_[X] > _shapeB.max_[X]) {
        return false;
    }
    if (_shapeA.max_[Y] < _shapeB.min_[Y] || _shapeA.min_[Y] > _shapeB.max_[Y]) {
        return false;
    }
    if (_shapeA.max_[Z] < _shapeB.min_[Z] || _shapeA.min_[Z] > _shapeB.max_[Z]) {
        return false;
    }

    bool aIsPushBack = _aInfo && _aInfo->getPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _bInfo && _bInfo->getPushBackType() != CollisionPushBackType::None;

    if (!aIsPushBack && !bIsPushBack) {
        return true; // 衝突情報がない場合は何もしない
    }

    // 各軸ごとの重なり幅を計算
    float overlapMinX = (std::max)(_shapeA.min_[X], _shapeB.min_[X]);
    float overlapMaxX = (std::min)(_shapeA.max_[X], _shapeB.max_[X]);

    float overlapMinY = (std::max)(_shapeA.min_[Y], _shapeB.min_[Y]);
    float overlapMaxY = (std::min)(_shapeA.max_[Y], _shapeB.max_[Y]);

    float overlapMinZ = (std::max)(_shapeA.min_[Z], _shapeB.min_[Z]);
    float overlapMaxZ = (std::min)(_shapeA.max_[Z], _shapeB.max_[Z]);

    // 各軸ごとの重なり区間
    float overlapX = overlapMaxX - overlapMinX;
    float overlapY = overlapMaxY - overlapMinY;
    float overlapZ = overlapMaxZ - overlapMinZ;

    // 最小の重なり軸を探す
    float minOverlap = overlapX;
    int axis         = X;
    if (overlapY < minOverlap) {
        minOverlap = overlapY;
        axis       = Y;
    }
    if (overlapZ < minOverlap) {
        minOverlap = overlapZ;
        axis       = Z;
    }

    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f aHalfSize = (_shapeA.max_ - _shapeA.min_) * 0.5f;
    Vec3f bHalfSize = (_shapeB.max_ - _shapeB.min_) * 0.5f;
    Vec3f aCenter   = _shapeA.min_ + aHalfSize;
    Vec3f bCenter   = _shapeB.min_ + bHalfSize;

    // 押し出し方向を決定
    float dir = (aCenter[axis] < bCenter[axis]) ? -1.0f : 1.0f;

    // collVecを作成
    Vec3f collVec(0, 0, 0);
    collVec[axis] = (minOverlap + 0.0001f) * dir;

    // 重なり部分の中心＝衝突点
    Vec3f collPoint = {
        overlapMinX + (overlapMaxX - overlapMinX) * overlapRate,
        overlapMinY + (overlapMaxY - overlapMinY) * overlapRate,
        overlapMinZ + (overlapMaxZ - overlapMinZ) * overlapRate};

    if (aIsPushBack) {
        // 衝突時の処理
        CollisionPushBackInfo::Info info;
        info.collVec       = Vec3f(0, 0, 0);
        info.collVec[axis] = (minOverlap * overlapRate) * dir;

        info.collPoint = collPoint;

        _aInfo->AddCollisionInfo(_entityB->getID(), info);
    }

    if (bIsPushBack) {
        CollisionPushBackInfo::Info info;
        info.collVec       = Vec3f(0, 0, 0);
        info.collVec[axis] = (minOverlap * overlapRate) * -dir;

        info.collPoint = collPoint;

        _bInfo->AddCollisionInfo(_entityA->getID(), info);
    }

    return true;
}

template <>
bool CheckCollisionPair(
    GameEntity* _aabbEntity,
    GameEntity* _sphereEntity,
    const AABB& _aabb,
    const Sphere& _sphere,
    [[maybe_unused]] CollisionPushBackInfo* _aabbInfo,
    [[maybe_unused]] CollisionPushBackInfo* _sphereInfo) {
    Vec3f closest = {
        std::clamp(_sphere.center_[X], _aabb.min_[X], _aabb.max_[X]),
        std::clamp(_sphere.center_[Y], _aabb.min_[Y], _aabb.max_[Y]),
        std::clamp(_sphere.center_[Z], _aabb.min_[Z], _aabb.max_[Z])};

    Vec3f distance = closest - _sphere.center_;

    if (distance.lengthSq() >= _sphere.radius_ * _sphere.radius_) {
        return false;
    }

    bool aabbIsPushBack   = _aabbInfo && _aabbInfo->getPushBackType() != CollisionPushBackType::None;
    bool sphereIsPushBack = _sphereInfo && _sphereInfo->getPushBackType() != CollisionPushBackType::None;

    if (!aabbIsPushBack && !sphereIsPushBack) {
        return true; // 衝突情報がない場合は何もしない
    }

    float overlapRate = 1.f / (float(aabbIsPushBack) + float(sphereIsPushBack));

    if (aabbIsPushBack) {
        // 衝突時の処理
        CollisionPushBackInfo::Info info;
        info.collPoint = _sphere.center_ + closest.normalize() * _sphere.radius_;
        info.collVec   = (distance.normalize() * (_sphere.radius_ - distance.length())) * overlapRate;

        _aabbInfo->AddCollisionInfo(_sphereEntity->getID(), info);
    }

    if (sphereIsPushBack) {
        Vec3f normal(0, 0, 0);
        Vec3f diff = _sphere.center_ - closest;
        float absX = std::abs(diff[X]);
        float absY = std::abs(diff[Y]);
        float absZ = std::abs(diff[Z]);

        if (absX >= absY && absX >= absZ) {
            normal[X] = (diff[X] > 0) ? 1.0f : -1.0f;
        } else if (absY >= absX && absY >= absZ) {
            normal[Y] = (diff[Y] > 0) ? 1.0f : -1.0f;
        } else {
            normal[Z] = (diff[Z] > 0) ? 1.0f : -1.0f;
        }

        CollisionPushBackInfo::Info info;
        info.collPoint = closest;
        info.collVec   = normal * ((_sphere.radius_ - distance.length() * overlapRate));

        _sphereInfo->AddCollisionInfo(_aabbEntity->getID(), info);
    }

    return true;
};

template <>
bool CheckCollisionPair(
    GameEntity* _entityA,
    GameEntity* _entityB,
    // 順番が A, B と B, A で同じ処理を行うため、関数を呼び出しを入れ替える
    const Sphere& _shapeA,
    const AABB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<AABB, Sphere>(_entityB, _entityA, _shapeB, _shapeA, _bInfo, _aInfo);
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
        if (transform) {
            transform->Update();
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

void CollisionCheckSystem::UpdateEntity(GameEntity* _entity) {
    ++entityItr_;

    auto aCollPushbackInfo       = getComponent<CollisionPushBackInfo>(_entity);
    auto* aEntityAabbColliders   = getComponents<AABBCollider>(_entity);
    auto* aEntitySphereColliders = getComponents<SphereCollider>(_entity);

    // 2つのリスト間の衝突判定をまとめる
    auto checkCollisions = [&](
                               GameEntity* aEntity,
                               GameEntity* bEntity,
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
        GameEntity* bEntity = *bItr;

        auto bCollPushbackInfo      = getComponent<CollisionPushBackInfo>(bEntity);
        auto bEntityAabbColliders   = getComponents<AABBCollider>(bEntity);
        auto bEntitySphereColliders = getComponents<SphereCollider>(bEntity);

        if (aEntityAabbColliders) {
            if (bEntityAabbColliders) {
                checkCollisions(_entity, bEntity, aEntityAabbColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (bEntitySphereColliders) {
                checkCollisions(_entity, bEntity, aEntityAabbColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
        }
        if (aEntitySphereColliders) {
            if (bEntityAabbColliders) {
                checkCollisions(_entity, bEntity, aEntitySphereColliders, bEntityAabbColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
            if (bEntitySphereColliders) {
                checkCollisions(_entity, bEntity, aEntitySphereColliders, bEntitySphereColliders, aCollPushbackInfo, bCollPushbackInfo);
            }
        }
    }
}
