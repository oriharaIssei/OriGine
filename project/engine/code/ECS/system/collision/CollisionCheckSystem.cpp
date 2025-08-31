#include "CollisionCheckSystem.h"

/// stl
#include <algorithm>

/// ECS

// component
#include "component/collider/Collider.h"
#include "component/collider/CollisionPushBackInfo.h"
#include "component/transform/Transform.h"

#pragma region "CheckCollisionPair"

constexpr float kEpsilon = 1e-6f;

template <typename ShapeA, typename ShapeB>
bool CheckCollisionPair(
    [[maybe_unused]] GameEntity* _entityA,
    [[maybe_unused]] GameEntity* _entityB,
    [[maybe_unused]] const ShapeA& _shapeA,
    [[maybe_unused]] const ShapeB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {
    return false;
}

/// <summary>
/// Sphere vs Sphere
/// </summary>
/// <typeparam name="ShapeA"></typeparam>
/// <typeparam name="ShapeB"></typeparam>
/// <param name="_entityA"></param>
/// <param name="_entityB"></param>
/// <param name="_shapeA"></param>
/// <param name="_shapeB"></param>
/// <param name="_aInfo"></param>
/// <param name="_bInfo"></param>
/// <returns></returns>
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

    if (!_aInfo || !_bInfo) {
        return true;
    }

    bool aIsPushBack = _bInfo->getPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _aInfo->getPushBackType() != CollisionPushBackType::None;

    if (!aIsPushBack && !bIsPushBack) {
        return true; // 衝突情報がない場合は何もしない
    }

    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collNormal      = distance.normalize();
    float overlapDistance = (_shapeA.radius_ + _shapeB.radius_) - distance.length();

    // 衝突時の処理
    CollisionPushBackInfo::Info aInfo;
    aInfo.collVec   = collNormal * overlapDistance * overlapRate;
    aInfo.collPoint = _shapeA.center_ + aInfo.collVec.normalize() * _shapeA.radius_;
    _aInfo->AddCollisionInfo(_entityB->getID(), aInfo);

    // 衝突時の処理
    CollisionPushBackInfo::Info bInfo;
    bInfo.collVec   = -collNormal * overlapDistance * overlapRate;
    bInfo.collPoint = _shapeB.center_ + bInfo.collVec.normalize() * _shapeB.radius_;
    _bInfo->AddCollisionInfo(_entityA->getID(), bInfo);

    return true;
}

/// <summary>
/// Sphere vs AABB
/// </summary>
/// <typeparam name="ShapeA"></typeparam>
/// <typeparam name="ShapeB"></typeparam>
/// <param name="_entityA"></param>
/// <param name="_entityB"></param>
/// <param name="_shapeA"></param>
/// <param name="_shapeB"></param>
/// <param name="_aInfo"></param>
/// <param name="_bInfo"></param>
/// <returns></returns>
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

    if (!_aabbInfo || !_sphereInfo) {
        return true;
    }

    bool aabbIsPushBack   = _sphereInfo && _sphereInfo->getPushBackType() != CollisionPushBackType::None;
    bool sphereIsPushBack = _aabbInfo && _aabbInfo->getPushBackType() != CollisionPushBackType::None;

    float overlapRate = 1.f / (float(aabbIsPushBack) + float(sphereIsPushBack));

    // 衝突時の処理
    CollisionPushBackInfo::Info aInfo;
    aInfo.collPoint = _sphere.center_ + closest.normalize() * _sphere.radius_;
    aInfo.collVec   = (distance.normalize() * (_sphere.radius_ - distance.length())) * overlapRate;

    _aabbInfo->AddCollisionInfo(_sphereEntity->getID(), aInfo);

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

    CollisionPushBackInfo::Info bInfo;
    bInfo.collPoint = closest;
    bInfo.collVec   = normal * ((_sphere.radius_ - distance.length() * overlapRate));

    _sphereInfo->AddCollisionInfo(_aabbEntity->getID(), bInfo);

    return true;
};

/// <summary>
///  Sphere vs AABB
/// </summary>
/// <typeparam name="ShapeA"></typeparam>
/// <typeparam name="ShapeB"></typeparam>
/// <param name="_entityA"></param>
/// <param name="_entityB"></param>
/// <param name="_shapeA"></param>
/// <param name="_shapeB"></param>
/// <param name="_aInfo"></param>
/// <param name="_bInfo"></param>
/// <returns></returns>
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

/// <summary>
/// Sphere vs OBB
/// </summary>
template <>
bool CheckCollisionPair(
    GameEntity* _entityA,
    GameEntity* _entityB,
    const Sphere& _shapeA,
    const OBB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {
    auto& obb    = _shapeB;
    auto& sphere = _shapeA;

    const Vec3f& obbCenter    = obb.center_;
    const Vec3f& sphereCenter = sphere.center_;
    const Vec3f& obbHalfSize  = obb.halfSize_;

    // --- Sphere中心を OBBローカル座標 へ ---
    Vec3f localCenter = sphereCenter - obbCenter;
    auto invRot       = obb.orientations_.rot.Conjugation();
    localCenter       = (MakeMatrix::RotateQuaternion(invRot) * MakeMatrix::Translate(localCenter))[3];

    // --- localAABB との判定 ---
    Vec3f aabbMin = -obbHalfSize;
    Vec3f aabbMax = obbHalfSize;

    // 最近接点
    Vec3f closest = {
        std::clamp(localCenter[X], aabbMin[X], aabbMax[X]),
        std::clamp(localCenter[Y], aabbMin[Y], aabbMax[Y]),
        std::clamp(localCenter[Z], aabbMin[Z], aabbMax[Z])};

    Vec3f diff   = localCenter - closest;
    float distSq = diff.lengthSq();

    if (distSq > sphere.radius_ * sphere.radius_) {
        return false; // 衝突なし
    }

    // --- 衝突ベクトルをローカルで計算 ---
    float dist = std::sqrt(distSq);
    Vec3f normal;
    if (dist > 1e-6f) {
        normal = diff / dist;
    } else {
        normal = Vec3f(1, 0, 0); // 球がAABB内部に完全に埋まってるときのfallback
    }

    float penetration  = sphere.radius_ - dist;
    Vec3f localCollVec = normal * penetration;

    Vec3f localCollPoint = closest; // 最近接点を衝突点とする

    // --- ワールドに戻す ---
    auto rotMat          = MakeMatrix::RotateQuaternion(obb.orientations_.rot);
    Vec3f worldCollVec   = localCollVec * rotMat;
    Vec3f worldCollPoint = obbCenter + (localCollPoint * rotMat);

    // --- PushBack 情報を登録 ---
    if (!_aInfo || !_bInfo) {
        return true;
    }
    bool aIsPushBack = _aInfo->getPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _bInfo->getPushBackType() != CollisionPushBackType::None;

    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f direction = worldCollVec.normalize();
    float length    = worldCollVec.length();

    CollisionPushBackInfo::Info aInfo;
    aInfo.collVec   = direction * (length * overlapRate);
    aInfo.collPoint = worldCollPoint;
    _aInfo->AddCollisionInfo(_entityB->getID(), aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.collVec   = direction * -(length * overlapRate);
    bInfo.collPoint = worldCollPoint;
    _bInfo->AddCollisionInfo(_entityA->getID(), bInfo);

    return true;
};

/// <summary>
/// Sphere vs OBB
/// </summary>
template <>
bool CheckCollisionPair(
    GameEntity* _entityA,
    GameEntity* _entityB,
    // 順番が A, B と B, A で同じ処理を行うため、関数を呼び出しを入れ替える
    const OBB& _shapeA,
    const Sphere& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Sphere, OBB>(_entityB, _entityA, _shapeB, _shapeA, _bInfo, _aInfo);
};

/// <summary>
/// AABB vs AABB
/// </summary>
/// <typeparam name="ShapeA"></typeparam>
/// <typeparam name="ShapeB"></typeparam>
/// <param name="_entityA"></param>
/// <param name="_entityB"></param>
/// <param name="_shapeA"></param>
/// <param name="_shapeB"></param>
/// <param name="_aInfo"></param>
/// <param name="_bInfo"></param>
/// <returns></returns>
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

    if (!_aInfo || !_bInfo) {
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

    bool aIsPushBack = _aInfo->getPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _bInfo->getPushBackType() != CollisionPushBackType::None;

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

    // 衝突時の処理
    CollisionPushBackInfo::Info ainfo;
    ainfo.collVec       = Vec3f(0, 0, 0);
    ainfo.collVec[axis] = (minOverlap * overlapRate) * dir;

    ainfo.collPoint = collPoint;

    _aInfo->AddCollisionInfo(_entityB->getID(), ainfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.collVec       = Vec3f(0, 0, 0);
    bInfo.collVec[axis] = (minOverlap * overlapRate) * -dir;

    bInfo.collPoint = collPoint;

    _bInfo->AddCollisionInfo(_entityA->getID(), bInfo);

    return true;
}

template <>
bool CheckCollisionPair(
    GameEntity* _entityA,
    GameEntity* _entityB,
    const OBB& _shapeA,
    const OBB& _shapeB,
    CollisionPushBackInfo* _aInfo,
    CollisionPushBackInfo* _bInfo) {
    // === 頂点計算 ===
    auto computeVerts = [](const OBB& obb) {
        std::array<Vec3f, 8> verts;
        Vec3f min     = -obb.halfSize_;
        Vec3f max     = obb.halfSize_;
        Matrix4x4 mat = MakeMatrix::Affine({1.f, 1.f, 1.f}, obb.orientations_.rot, obb.center_);
        verts[0]      = TransformVector({min[X], min[Y], min[Z]}, mat);
        verts[1]      = TransformVector({max[X], min[Y], min[Z]}, mat);
        verts[2]      = TransformVector({min[X], max[Y], min[Z]}, mat);
        verts[3]      = TransformVector({max[X], max[Y], min[Z]}, mat);
        verts[4]      = TransformVector({min[X], min[Y], max[Z]}, mat);
        verts[5]      = TransformVector({max[X], min[Y], max[Z]}, mat);
        verts[6]      = TransformVector({min[X], max[Y], max[Z]}, mat);
        verts[7]      = TransformVector({max[X], max[Y], max[Z]}, mat);
        return verts;
    };
    auto vertsA = computeVerts(_shapeA);
    auto vertsB = computeVerts(_shapeB);

    // === 軸候補 ===
    std::vector<Vec3f> axes;
    for (int i = 0; i < 3; i++) {
        axes.push_back(_shapeA.orientations_.axis[i]);
    }
    for (int i = 0; i < 3; i++) {
        axes.push_back(_shapeB.orientations_.axis[i]);
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            Vec3f crossAxis = _shapeA.orientations_.axis[i].cross(_shapeB.orientations_.axis[j]);
            if (crossAxis.lengthSq() > 1e-6f)
                axes.push_back(crossAxis.normalize());
        }
    }

    // === SAT 判定 & 最小overlap ===
    float minOverlap = FLT_MAX;
    Vec3f minAxis(0, 0, 0);

    auto project = [](const std::array<Vec3f, 8>& verts, const Vec3f& axis, float& outMin, float& outMax) {
        float p = verts[0].dot(axis);
        outMin = outMax = p;
        for (int i = 1; i < 8; i++) {
            float v = verts[i].dot(axis);
            if (v < outMin) {
                outMin = v;
            }
            if (v > outMax) {
                outMax = v;
            }
        }
    };

    for (auto& axisRaw : axes) {
        Vec3f axis = axisRaw.normalize();
        float minA, maxA, minB, maxB;
        project(vertsA, axis, minA, maxA);
        project(vertsB, axis, minB, maxB);
        float overlap = std::min(maxA, maxB) - std::max(minA, minB);
        if (overlap <= 0) {
            return false; // 分離
        }
        if (overlap < minOverlap) {
            minOverlap = overlap;
            minAxis    = axis;
        }
    }

    // === サポート点で法線方向を決定 ===
    auto supportPoint = [](const std::array<Vec3f, 8>& verts, const Vec3f& dir) {
        float maxProj = -FLT_MAX;
        Vec3f best;
        for (auto& v : verts) {
            float p = v.dot(dir);
            if (p > maxProj) {
                maxProj = p;
                best    = v;
            }
        }
        return best;
    };

    Vec3f pA = supportPoint(vertsA, minAxis);
    Vec3f pB = supportPoint(vertsB, -minAxis);

    // 法線の向きを補正
    Vec3f collNormal = minAxis;
    if (Vec3f(pB - pA).dot(collNormal) < 0) {
        collNormal = -collNormal;
    }

    // penetration
    float penetration = minOverlap;

    // 衝突点 = サポート点の平均
    Vec3f collPoint = (pA + pB) * 0.5f;

    // collVec = 法線 * penetration
    Vec3f collVec = collNormal * penetration;

    // === PushBack 情報 ===
    if (!_aInfo || !_bInfo) {
        return true;
    }
    bool aIsPushBack = _aInfo->getPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _bInfo->getPushBackType() != CollisionPushBackType::None;

    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    CollisionPushBackInfo::Info ainfo;
    ainfo.collVec   = -collVec * overlapRate;
    ainfo.collPoint = collPoint;
    _aInfo->AddCollisionInfo(_entityB->getID(), ainfo);

    CollisionPushBackInfo::Info binfo;
    binfo.collVec   = collVec * overlapRate;
    binfo.collPoint = collPoint;
    _bInfo->AddCollisionInfo(_entityA->getID(), binfo);

    return true;
}

/// <summary>
/// AABB vs OBB
/// </summary>
template <>
bool CheckCollisionPair(
    GameEntity* _entityA,
    GameEntity* _entityB,
    const AABB& _shapeA,
    const OBB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {

    // --- AABB を OBB と同じ形式に変換 ---
    Vec3f aCenter   = (_shapeA.min_ + _shapeA.max_) * 0.5f;
    Vec3f aHalfSize = (_shapeA.max_ - _shapeA.min_) * 0.5f;

    OBB aAsOBB;
    aAsOBB.center_       = aCenter;
    aAsOBB.halfSize_     = aHalfSize;
    aAsOBB.orientations_ = Orientation::Identity();

    // --- OBB vs OBB 判定を使う ---
    return CheckCollisionPair<OBB, OBB>(_entityA, _entityB, aAsOBB, _shapeB, _aInfo, _bInfo);
}

/// <summary>
/// OBB vs AABB
/// </summary>
template <>
bool CheckCollisionPair(
    GameEntity* _entityA,
    GameEntity* _entityB,
    const OBB& _shapeA,
    const AABB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {
    // 順序を入れ替えて再利用
    return CheckCollisionPair<AABB, OBB>(_entityB, _entityA, _shapeB, _shapeA, _bInfo, _aInfo);
}

#pragma endregion

CollisionCheckSystem::CollisionCheckSystem()
    : ISystem(SystemCategory::Collision) {
}

CollisionCheckSystem::~CollisionCheckSystem() {}

void CollisionCheckSystem::Initialize() {
    entities_.reserve(100);
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

void CollisionCheckSystem::UpdateEntity(GameEntity* _entity) {
    ++entityItr_;

    auto aCollPushbackInfo       = getComponent<CollisionPushBackInfo>(_entity);
    auto* aEntityAabbColliders   = getComponents<AABBCollider>(_entity);
    auto* aEntitySphereColliders = getComponents<SphereCollider>(_entity);
    auto* aEntityObbColliders    = getComponents<OBBCollider>(_entity);

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
