#pragma once

#include "base/IBounds.h"

/// math
#include "Vector3.h"

namespace OriGine {
namespace Bounds {

/// <summary>
/// 軸平行境界ボックス
/// Axis Aligned Bounding Box
/// </summary>
struct AABB
    : public IBounds {
    AABB() {}
    AABB(const Vec3f& _center, const Vec3f& _halfSize) : center(_center), halfSize(_halfSize) {}

    Vec3f center   = {0.f, 0.f, 0.f};
    Vec3f halfSize = {0.f, 0.f, 0.f};

    Vec3f Min() const { return center - halfSize; }
    Vec3f Max() const { return center + halfSize; };
};

} // namespace Bounds
} // namespace OriGine
