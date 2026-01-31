#include "CollisionCheckPairFunc.h"
#include "CollisionCheckUtility.h"

/// math
#include "math/mathEnv.h"
#include "math/Vector3.h"

namespace OriGine {

#pragma region Segment Collisions

/// <summary>
/// Segment vs Sphere の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Segment& _shapeA, const Bounds::Sphere& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f closest = ClosestPointOnSegment(_shapeB.center_, _shapeA.start, _shapeA.end);
    Vec3f diff    = _shapeB.center_ - closest;
    float distSq  = diff.lengthSq();

    if (distSq > _shapeB.radius_ * _shapeB.radius_) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f normal      = diff.normalize();

    float dist        = std::sqrt(distSq);
    float penetration = _shapeB.radius_ - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = closest;
    aInfo.collFaceNormal = -normal;
    aInfo.collVec        = -normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = closest;
    bInfo.collFaceNormal = normal;
    bInfo.collVec        = normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// Sphere vs Segment の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Sphere& _shapeA, const Bounds::Segment& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Segment, Bounds::Sphere>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Segment vs AABB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Segment& _shapeA, const Bounds::AABB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f aabbMin = _shapeB.Min();
    Vec3f aabbMax = _shapeB.Max();
    Vec3f dir     = _shapeA.end - _shapeA.start;
    float length  = dir.length();

    if (length < kEpsilon) {
        // 線分が点に縮退
        Vec3f p = _shapeA.start;
        if (p[X] >= aabbMin[X] && p[X] <= aabbMax[X] && p[Y] >= aabbMin[Y] && p[Y] <= aabbMax[Y] && p[Z] >= aabbMin[Z] && p[Z] <= aabbMax[Z]) {
            if (_aInfo && _bInfo) {
                CollisionPushBackInfo::Info aInfo;
                aInfo.pushBackType = _bInfo->GetPushBackType();
                aInfo.collPoint    = p;
                aInfo.collVec      = Vec3f(0, 0, 0);
                _aInfo->AddCollisionInfo(_handleB, aInfo);

                CollisionPushBackInfo::Info bInfo;
                bInfo.pushBackType = _aInfo->GetPushBackType();
                bInfo.collPoint    = p;
                bInfo.collVec      = Vec3f(0, 0, 0);
                _bInfo->AddCollisionInfo(_handleA, bInfo);
            }
            return true;
        }
        return false;
    }

    dir = dir / length;

    float tMin  = 0.f;
    float tMax  = length;
    int hitAxis = -1;
    int hitSign = 0;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(dir[i]) < kEpsilon) {
            if (_shapeA.start[i] < aabbMin[i] || _shapeA.start[i] > aabbMax[i]) {
                return false;
            }
        } else {
            float ood = 1.0f / dir[i];
            float t1  = (aabbMin[i] - _shapeA.start[i]) * ood;
            float t2  = (aabbMax[i] - _shapeA.start[i]) * ood;
            int sign  = 1;
            if (t1 > t2) {
                std::swap(t1, t2);
                sign = -1;
            }
            if (t1 > tMin) {
                tMin    = t1;
                hitAxis = i;
                hitSign = sign;
            }
            if (t2 < tMax) {
                tMax = t2;
            }
            if (tMin > tMax) {
                return false;
            }
        }
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f hitPoint = _shapeA.start + dir * tMin;
    Vec3f normal(0, 0, 0);
    if (hitAxis >= 0) {
        normal[hitAxis] = static_cast<float>(-hitSign);
    }

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = hitPoint;
    aInfo.collFaceNormal = -normal;
    aInfo.collVec        = Vec3f(0, 0, 0);
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = hitPoint;
    bInfo.collFaceNormal = normal;
    bInfo.collVec        = Vec3f(0, 0, 0);
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// AABB vs Segment の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::AABB& _shapeA, const Bounds::Segment& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Segment, Bounds::AABB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Segment vs OBB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Segment& _shapeA, const Bounds::OBB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // OBBのローカル空間に変換
    Vec3f localStart = _shapeA.start - _shapeB.center_;
    Vec3f localEnd   = _shapeA.end - _shapeB.center_;
    Vec3f segStart, segEnd;

    for (int i = 0; i < 3; ++i) {
        segStart[i] = localStart.dot(_shapeB.orientations_.axis[i]);
        segEnd[i]   = localEnd.dot(_shapeB.orientations_.axis[i]);
    }

    Vec3f dir    = segEnd - segStart;
    float length = dir.length();

    if (length < kEpsilon) {
        // 点と判定
        for (int i = 0; i < 3; ++i) {
            if (segStart[i] < -_shapeB.halfSize_[i] || segStart[i] > _shapeB.halfSize_[i]) {
                return false;
            }
        }
        if (_aInfo && _bInfo) {
            CollisionPushBackInfo::Info aInfo;
            aInfo.pushBackType = _bInfo->GetPushBackType();
            aInfo.collPoint    = _shapeA.start;
            aInfo.collVec      = Vec3f(0, 0, 0);
            _aInfo->AddCollisionInfo(_handleB, aInfo);

            CollisionPushBackInfo::Info bInfo;
            bInfo.pushBackType = _aInfo->GetPushBackType();
            bInfo.collPoint    = _shapeA.start;
            bInfo.collVec      = Vec3f(0, 0, 0);
            _bInfo->AddCollisionInfo(_handleA, bInfo);
        }
        return true;
    }

    dir = dir / length;

    float tMin  = 0.f;
    float tMax  = length;
    int hitAxis = -1;
    int hitSign = 0;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(dir[i]) < kEpsilon) {
            if (segStart[i] < -_shapeB.halfSize_[i] || segStart[i] > _shapeB.halfSize_[i]) {
                return false;
            }
        } else {
            float ood = 1.0f / dir[i];
            float t1  = (-_shapeB.halfSize_[i] - segStart[i]) * ood;
            float t2  = (_shapeB.halfSize_[i] - segStart[i]) * ood;
            int sign  = 1;
            if (t1 > t2) {
                std::swap(t1, t2);
                sign = -1;
            }
            if (t1 > tMin) {
                tMin    = t1;
                hitAxis = i;
                hitSign = sign;
            }
            if (t2 < tMax) {
                tMax = t2;
            }
            if (tMin > tMax) {
                return false;
            }
        }
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f localHit = segStart + dir * tMin;
    Vec3f hitPoint = _shapeB.center_;
    for (int i = 0; i < 3; ++i) {
        hitPoint = hitPoint + _shapeB.orientations_.axis[i] * localHit[i];
    }

    Vec3f normal(0, 0, 0);
    if (hitAxis >= 0) {
        normal = _shapeB.orientations_.axis[hitAxis] * static_cast<float>(-hitSign);
    }

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = hitPoint;
    aInfo.collFaceNormal = -normal;
    aInfo.collVec        = Vec3f(0, 0, 0);
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = hitPoint;
    bInfo.collFaceNormal = normal;
    bInfo.collVec        = Vec3f(0, 0, 0);
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// OBB vs Segment の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::OBB& _shapeA, const Bounds::Segment& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Segment, Bounds::OBB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Segment vs Segment の衝突判定の実装（接触判定）
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Segment& _shapeA, const Bounds::Segment& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f closest1, closest2;
    ClosestPointsBetweenSegments(_shapeA.start, _shapeA.end, _shapeB.start, _shapeB.end, closest1, closest2);

    Vec3f diff   = closest2 - closest1;
    float distSq = diff.lengthSq();

    // 線分同士は厳密な接触判定（距離がほぼ0の場合のみ衝突）
    if (distSq > kEpsilon * kEpsilon) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType = _bInfo->GetPushBackType();
    aInfo.collPoint    = closest1;
    aInfo.collVec      = Vec3f(0, 0, 0);
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType = _aInfo->GetPushBackType();
    bInfo.collPoint    = closest2;
    bInfo.collVec      = Vec3f(0, 0, 0);
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

#pragma endregion

} // namespace OriGine
