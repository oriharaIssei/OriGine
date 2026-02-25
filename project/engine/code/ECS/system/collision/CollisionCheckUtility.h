#pragma once

/// math
#include "math/bounds/AABB.h"
#include "math/bounds/OBB.h"
#include "math/MathEnv.h"
#include "math/Vector3.h"

namespace OriGine {

/// <summary>
/// 線分上の最近接点を求める（パラメータt: 0～1にクランプ）
/// </summary>
inline Vec3f ClosestPointOnSegment(const Vec3f& _point, const Vec3f& _segStart, const Vec3f& _segEnd) {
    Vec3f ab      = _segEnd - _segStart;
    float abLenSq = ab.lengthSq();
    if (abLenSq < kEpsilon) {
        return _segStart;
    }
    float t = Vec3f(_point - _segStart).dot(ab) / abLenSq;
    t       = std::clamp(t, 0.f, 1.f);
    return _segStart + ab * t;
}

/// <summary>
/// 2つの線分間の最近接点ペアを求める
/// </summary>
inline void ClosestPointsBetweenSegments(
    const Vec3f& _p1, const Vec3f& _q1, // 線分1
    const Vec3f& _p2, const Vec3f& _q2, // 線分2
    Vec3f& _closest1, Vec3f& _closest2) {

    Vec3f d1 = _q1 - _p1;
    Vec3f d2 = _q2 - _p2;
    Vec3f r  = _p1 - _p2;

    float a = d1.dot(d1);
    float e = d2.dot(d2);
    float f = d2.dot(r);

    float s, t;

    if (a < kEpsilon && e < kEpsilon) {
        // 両方の線分が点に縮退
        _closest1 = _p1;
        _closest2 = _p2;
        return;
    }

    if (a < kEpsilon) {
        // 線分1が点に縮退
        s = 0.f;
        t = std::clamp(f / e, 0.f, 1.f);
    } else {
        float c = d1.dot(r);
        if (e < kEpsilon) {
            // 線分2が点に縮退
            t = 0.f;
            s = std::clamp(-c / a, 0.f, 1.f);
        } else {
            // 一般ケース
            float b     = d1.dot(d2);
            float denom = a * e - b * b;

            if (denom != 0.f) {
                s = std::clamp((b * f - c * e) / denom, 0.f, 1.f);
            } else {
                s = 0.f;
            }

            t = (b * s + f) / e;

            if (t < 0.f) {
                t = 0.f;
                s = std::clamp(-c / a, 0.f, 1.f);
            } else if (t > 1.f) {
                t = 1.f;
                s = std::clamp((b - c) / a, 0.f, 1.f);
            }
        }
    }

    _closest1 = _p1 + d1 * s;
    _closest2 = _p2 + d2 * t;
}

/// <summary>
/// AABBと点の最近接点を求める
/// </summary>
inline Vec3f ClosestPointOnAABB(const Vec3f& _point, const Bounds::AABB& _aabb) {
    Vec3f aabbMin = _aabb.Min();
    Vec3f aabbMax = _aabb.Max();
    return {
        std::clamp(_point[X], aabbMin[X], aabbMax[X]),
        std::clamp(_point[Y], aabbMin[Y], aabbMax[Y]),
        std::clamp(_point[Z], aabbMin[Z], aabbMax[Z])};
}

/// <summary>
/// OBBと点の最近接点を求める
/// </summary>
inline Vec3f ClosestPointOnOBB(const Vec3f& _point, const Bounds::OBB& _obb) {
    Vec3f d      = _point - _obb.center_;
    Vec3f result = _obb.center_;

    for (int i = 0; i < 3; ++i) {
        float dist = d.dot(_obb.orientations_.axis[i]);
        dist       = std::clamp(dist, -_obb.halfSize_[i], _obb.halfSize_[i]);
        result     = result + _obb.orientations_.axis[i] * dist;
    }

    return result;
}

} // namespace OriGine
