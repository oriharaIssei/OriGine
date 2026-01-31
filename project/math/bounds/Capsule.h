#pragma once

#include "base/IBounds.h"

/// math
#include "Vector3.h"

/// bounds
#include "Segment.h"

namespace OriGine {
namespace Bounds {

/// <summary>
/// カプセル
/// 線分を太さ（半径）で膨らませた形状
/// </summary>
struct Capsule
    : public IBounds {
    Capsule() {}
    Capsule(const Vec3f& _start, const Vec3f& _end, float _radius)
        : segment(_start, _end), radius(_radius) {}
    Capsule(const Segment& _segment, float _radius)
        : segment(_segment), radius(_radius) {}

    /// <summary>
    /// 中心軸となる線分
    /// </summary>
    Segment segment;
    /// <summary>
    /// 半径
    /// </summary>
    float radius = 0.f;

    /// <summary>
    /// カプセルの全長を取得（両端の半球を含む）
    /// </summary>
    float TotalLength() const { return segment.Length() + radius * 2.f; }

    /// <summary>
    /// 線分の始点側の球の中心を取得
    /// </summary>
    Vec3f StartCenter() const { return segment.start; }

    /// <summary>
    /// 線分の終点側の球の中心を取得
    /// </summary>
    Vec3f EndCenter() const { return segment.end; }

    /// <summary>
    /// カプセルの中心を取得
    /// </summary>
    Vec3f Center() const { return segment.Center(); }
};

} // namespace Bounds
} // namespace OriGine
