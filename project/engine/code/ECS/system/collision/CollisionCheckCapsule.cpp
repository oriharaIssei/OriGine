#include "CollisionCheckPairFunc.h"
#include "CollisionCheckUtility.h"

/// math
#include "math/MathEnv.h"
#include "math/Vector3.h"

namespace OriGine {

#pragma region Capsule Collisions

/// <summary>
/// Capsule vs Capsule の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Capsule& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f closest1, closest2;
    ClosestPointsBetweenSegments(
        _shapeA.segment.start, _shapeA.segment.end,
        _shapeB.segment.start, _shapeB.segment.end,
        closest1, closest2);

    Vec3f diff      = closest2 - closest1;
    float distSq    = diff.lengthSq();
    float radiusSum = _shapeA.radius + _shapeB.radius;

    if (distSq > radiusSum * radiusSum) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f normal = diff.normalize();

    float dist        = std::sqrt(distSq);
    float penetration = radiusSum - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collPoint = closest1 + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// Capsule vs Sphere の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Capsule& _shapeA, const Bounds::Sphere& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f closest   = ClosestPointOnSegment(_shapeB.center_, _shapeA.segment.start, _shapeA.segment.end);
    Vec3f diff      = _shapeB.center_ - closest;
    float distSq    = diff.lengthSq();
    float radiusSum = _shapeA.radius + _shapeB.radius_;

    if (distSq > radiusSum * radiusSum) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f normal      = diff.normalize();
    float dist        = std::sqrt(distSq);
    float penetration = radiusSum - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collPoint = closest + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// Sphere vs Capsule の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Sphere& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Capsule, Bounds::Sphere>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Capsule vs AABB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Capsule& _shapeA, const Bounds::AABB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // カプセルの線分上の各点とAABBの最近接点を求め、最短距離を計算
    Vec3f closestOnSeg, closestOnAABB;
    float minDistSq = FLT_MAX;

    // 線分上の点をサンプリングして最近接点を求める
    const int numSamples = 10;
    for (int i = 0; i <= numSamples; ++i) {
        float t           = static_cast<float>(i) / numSamples;
        Vec3f pointOnSeg  = _shapeA.segment.GetPoint(t);
        Vec3f pointOnAABB = ClosestPointOnAABB(pointOnSeg, _shapeB);
        float distSq      = Vec3f(pointOnAABB - pointOnSeg).lengthSq();
        if (distSq < minDistSq) {
            minDistSq     = distSq;
            closestOnSeg  = pointOnSeg;
            closestOnAABB = pointOnAABB;
        }
    }

    if (minDistSq > _shapeA.radius * _shapeA.radius) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    float dist        = std::sqrt(minDistSq);
    Vec3f diff        = closestOnAABB - closestOnSeg;
    Vec3f normal      = diff.normalize();
    float penetration = _shapeA.radius - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collPoint = closestOnSeg + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// AABB vs Capsule の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::AABB& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Capsule, Bounds::AABB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Capsule vs OBB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Capsule& _shapeA, const Bounds::OBB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // カプセルの線分上の各点とOBBの最近接点を求め、最短距離を計算
    Vec3f closestOnSeg, closestOnOBB;
    float minDistSq = FLT_MAX;

    const int numSamples = 10;
    for (int i = 0; i <= numSamples; ++i) {
        float t          = static_cast<float>(i) / numSamples;
        Vec3f pointOnSeg = _shapeA.segment.GetPoint(t);
        Vec3f pointOnOBB = ClosestPointOnOBB(pointOnSeg, _shapeB);
        float distSq     = Vec3f(pointOnOBB - pointOnSeg).lengthSq();
        if (distSq < minDistSq) {
            minDistSq    = distSq;
            closestOnSeg = pointOnSeg;
            closestOnOBB = pointOnOBB;
        }
    }

    if (minDistSq > _shapeA.radius * _shapeA.radius) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    float dist        = std::sqrt(minDistSq);
    Vec3f diff        = closestOnOBB - closestOnSeg;
    Vec3f normal = diff.normalize();
    float penetration = _shapeA.radius - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collPoint = closestOnSeg + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// OBB vs Capsule の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::OBB& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Capsule, Bounds::OBB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Capsule vs Segment の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Capsule& _shapeA, const Bounds::Segment& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f closest1, closest2;
    ClosestPointsBetweenSegments(
        _shapeA.segment.start, _shapeA.segment.end,
        _shapeB.start, _shapeB.end,
        closest1, closest2);

    Vec3f diff   = closest2 - closest1;
    float distSq = diff.lengthSq();

    if (distSq > _shapeA.radius * _shapeA.radius) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    float dist        = std::sqrt(distSq);
    Vec3f normal = diff.normalize();
    float penetration = _shapeA.radius - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collPoint = closest1 + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// Segment vs Capsule の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Segment& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Capsule, Bounds::Segment>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

#pragma endregion

} // namespace OriGine
