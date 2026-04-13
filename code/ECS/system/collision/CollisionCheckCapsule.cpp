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
    // 反復最近接点法：線分上の点とAABB上の点を交互に更新して収束させる
    Vec3f closestOnSeg  = _shapeA.segment.Center();
    Vec3f closestOnAABB = ClosestPointOnAABB(closestOnSeg, _shapeB);

    constexpr int kMaxIterations = 8;
    for (int i = 0; i < kMaxIterations; ++i) {
        Vec3f prevOnSeg  = closestOnSeg;
        closestOnSeg     = ClosestPointOnSegment(closestOnAABB, _shapeA.segment.start, _shapeA.segment.end);
        closestOnAABB    = ClosestPointOnAABB(closestOnSeg, _shapeB);

        // 収束判定
        if (Vec3f(closestOnSeg - prevOnSeg).lengthSq() < kEpsilon * kEpsilon) {
            break;
        }
    }

    float minDistSq = Vec3f(closestOnAABB - closestOnSeg).lengthSq();

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
    // 反復最近接点法：線分上の点とOBB上の点を交互に更新して収束させる
    Vec3f closestOnSeg = _shapeA.segment.Center();
    Vec3f closestOnOBB = ClosestPointOnOBB(closestOnSeg, _shapeB);

    constexpr int kMaxIterations = 8;
    for (int i = 0; i < kMaxIterations; ++i) {
        Vec3f prevOnSeg = closestOnSeg;
        closestOnSeg    = ClosestPointOnSegment(closestOnOBB, _shapeA.segment.start, _shapeA.segment.end);
        closestOnOBB    = ClosestPointOnOBB(closestOnSeg, _shapeB);

        if (Vec3f(closestOnSeg - prevOnSeg).lengthSq() < kEpsilon * kEpsilon) {
            break;
        }
    }

    float minDistSq = Vec3f(closestOnOBB - closestOnSeg).lengthSq();

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
