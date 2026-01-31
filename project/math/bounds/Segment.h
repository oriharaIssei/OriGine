#pragma once

#include "base/IBounds.h"

/// math
#include "Vector3.h"

namespace OriGine {
namespace Bounds {

/// <summary>
/// 線分
/// 2点間を結ぶ有限の線
/// </summary>
struct Segment
    : public IBounds {
    Segment() {}
    Segment(const Vec3f& _start, const Vec3f& _end) : start(_start), end(_end) {}

    /// <summary>
    /// 始点
    /// </summary>
    Vec3f start = {0.f, 0.f, 0.f};
    /// <summary>
    /// 終点
    /// </summary>
    Vec3f end = {0.f, 0.f, 0.f};

    /// <summary>
    /// 線分の長さを取得
    /// </summary>
    float Length() const { return Vec3f(end - start).length(); }

    /// <summary>
    /// 線分の長さの2乗を取得
    /// </summary>
    float LengthSquared() const { return Vec3f(end - start).lengthSq(); }

    /// <summary>
    /// 線分の方向ベクトルを取得（正規化済み）
    /// </summary>
    Vec3f Direction() const { return Vec3f(end - start).normalize(); }

    /// <summary>
    /// 線分上の点を取得
    /// </summary>
    /// <param name="t">0.0～1.0の補間値</param>
    /// <returns>線分上の点</returns>
    Vec3f GetPoint(float t) const { return start + (end - start) * t; }

    /// <summary>
    /// 線分の中点を取得
    /// </summary>
    Vec3f Center() const { return (start + end) * 0.5f; }
};

} // namespace Bounds
} // namespace OriGine
