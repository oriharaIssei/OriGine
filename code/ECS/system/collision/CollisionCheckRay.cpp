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
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Ray& _shapeA, const Bounds::Sphere& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // レイ上の点 P(t) = origin + direction*t が球面上にある条件 |P(t) - center| = radius を
    // 展開すると、tについての2次方程式 a*t^2 + b*t + c = 0 になる。
    // よって交差判定は、この方程式が実数解を持つか(判別式が0以上か)に帰着する
    Vec3f oc           = _shapeA.origin - _shapeB.center_;
    float a            = _shapeA.direction.dot(_shapeA.direction);
    float b            = 2.0f * oc.dot(_shapeA.direction);
    float c            = oc.dot(oc) - _shapeB.radius_ * _shapeB.radius_;
    float discriminant = b * b - 4.f * a * c;

    // 判別式が負＝実数解なし＝レイは球をかすりもしない
    if (discriminant < 0.f) {
        return false;
    }

    // 解の公式。2つの解はレイが球に入る点と出る点にあたる。
    // 手前(小さい方)の解を優先し、それが負ならレイの始点が球の内部にあるということなので、
    // 出口側(大きい方)の解を採用する。両方負なら交点はレイの後方にあり、前方では当たらない
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
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Sphere& _shapeA, const Bounds::Ray& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Ray, Bounds::Sphere>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Ray vs AABB の衝突判定の実装（スラブ法）
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Ray& _shapeA, const Bounds::AABB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f aabbMin = _shapeB.Min();
    Vec3f aabbMax = _shapeB.Max();

    // スラブ法: AABBを3組の平行な板(スラブ)の共通部分とみなし、軸ごとに
    // レイが板へ入る時刻t1と出る時刻t2を求める。
    // 全軸の「入る時刻の最大値」tMinが「出る時刻の最小値」tMaxを超えたら、
    // 3枚の板を同時に貫いている区間が無い＝交差しない。
    // tMinの初期値が0なのはレイが始点より後方へ伸びないため。
    // レイには終端が無いのでtMaxはFLT_MAXから始める
    float tMin  = 0.f;
    float tMax  = FLT_MAX;
    // 最後にtMinを更新した軸が、最も遅く入った面＝実際に当たった面になる
    int hitAxis = -1;
    int hitSign = 0;

    for (int i = 0; i < 3; ++i) {
        // この軸方向の成分がほぼ0＝板に平行に進んでいるため出入りの時刻が定まらず、
        // 下の除算もゼロ除算になる。始点が板の外にあるなら永久に交差しない
        if (std::abs(_shapeA.direction[i]) < kEpsilon) {
            if (_shapeA.origin[i] < aabbMin[i] || _shapeA.origin[i] > aabbMax[i]) {
                return false;
            }
        } else {
            // ood = one over direction. 除算を1回にまとめる
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
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::AABB& _shapeA, const Bounds::Ray& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Ray, Bounds::AABB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Ray vs OBB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Ray& _shapeA, const Bounds::OBB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
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
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::OBB& _shapeA, const Bounds::Ray& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Ray, Bounds::OBB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

#pragma endregion

} // namespace OriGine
