#include "CollisionCheckPairFunc.h"
#include "CollisionCheckUtility.h"

/// math
#include "math/MathEnv.h"
#include "math/Vector3.h"

namespace OriGine {

#pragma region Ray Collisions

/// <summary>
/// Ray vs Sphere の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Ray& _shapeA, const Bounds::Sphere& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f oc           = _shapeA.origin - _shapeB.center_;
    float a            = _shapeA.direction.dot(_shapeA.direction);
    float b            = 2.0f * oc.dot(_shapeA.direction);
    float c            = oc.dot(oc) - _shapeB.radius_ * _shapeB.radius_;
    float discriminant = b * b - 4.f * a * c;

    if (discriminant < 0.f) {
        return false;
    }

    float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    if (t < 0.f) {
        t = (-b + std::sqrt(discriminant)) / (2.0f * a);
        if (t < 0.f) {
            return false;
        }
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f hitPoint = _shapeA.GetPoint(t);
    Vec3f normal   = Vec3f(hitPoint - _shapeB.center_).normalize();

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
/// Sphere vs Ray の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Sphere& _shapeA, const Bounds::Ray& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Ray, Bounds::Sphere>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Ray vs AABB の衝突判定の実装（スラブ法）
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Ray& _shapeA, const Bounds::AABB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f aabbMin = _shapeB.Min();
    Vec3f aabbMax = _shapeB.Max();

    float tMin  = 0.f;
    float tMax  = FLT_MAX;
    int hitAxis = -1;
    int hitSign = 0;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(_shapeA.direction[i]) < kEpsilon) {
            if (_shapeA.origin[i] < aabbMin[i] || _shapeA.origin[i] > aabbMax[i]) {
                return false;
            }
        } else {
            float ood = 1.0f / _shapeA.direction[i];
            float t1  = (aabbMin[i] - _shapeA.origin[i]) * ood;
            float t2  = (aabbMax[i] - _shapeA.origin[i]) * ood;
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

    if (tMin < 0.f) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f hitPoint = _shapeA.GetPoint(tMin);
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
/// AABB vs Ray の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::AABB& _shapeA, const Bounds::Ray& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Ray, Bounds::AABB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Ray vs OBB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, EntityHandle _handleA, EntityHandle _handleB, const Bounds::Ray& _shapeA, const Bounds::OBB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // OBBのローカル空間でレイを変換
    Vec3f localOrigin = _shapeA.origin - _shapeB.center_;
    Vec3f rayOrigin, rayDir;

    for (int i = 0; i < 3; ++i) {
        rayOrigin[i] = localOrigin.dot(_shapeB.orientations_.axis[i]);
        rayDir[i]    = _shapeA.direction.dot(_shapeB.orientations_.axis[i]);
    }

    float tMin  = 0.f;
    float tMax  = FLT_MAX;
    int hitAxis = -1;
    int hitSign = 0;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(rayDir[i]) < kEpsilon) {
            if (rayOrigin[i] < -_shapeB.halfSize_[i] || rayOrigin[i] > _shapeB.halfSize_[i]) {
                return false;
            }
        } else {
            float ood = 1.0f / rayDir[i];
            float t1  = (-_shapeB.halfSize_[i] - rayOrigin[i]) * ood;
            float t2  = (_shapeB.halfSize_[i] - rayOrigin[i]) * ood;
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

    if (tMin < 0.f) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f hitPoint = _shapeA.GetPoint(tMin);
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
/// OBB vs Ray の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, EntityHandle _handleA, EntityHandle _handleB, const Bounds::OBB& _shapeA, const Bounds::Ray& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Ray, Bounds::OBB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

#pragma endregion

} // namespace OriGine
