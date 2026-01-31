#pragma once

#include "base/IBounds.h"

/// math
#include "Vector3.h"

namespace OriGine {
namespace Bounds {

/// <summary>
/// レイ（半直線）
/// 始点から無限に伸びる線
/// </summary>
struct Ray
    : public IBounds {
    Ray() {}
    Ray(const Vec3f& _origin, const Vec3f& _direction) : origin(_origin), direction(_direction.normalize()) {}

    /// <summary>
    /// 始点
    /// </summary>
    Vec3f origin = {0.f, 0.f, 0.f};
    /// <summary>
    /// 方向（正規化済み）
    /// </summary>
    Vec3f direction = {0.f, 0.f, 1.f};

    /// <summary>
    /// レイ上の点を取得
    /// </summary>
    /// <param name="t">始点からの距離</param>
    /// <returns>レイ上の点</returns>
    Vec3f GetPoint(float t) const { return origin + direction * t; }
};

} // namespace Bounds
} // namespace OriGine
