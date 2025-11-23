#pragma once

#include "base/IBounds.h"

/// math
#include "Vector3.h"

namespace math::bounds {
struct AABB
    : public IBounds {
    AABB() {}
    AABB(const Vec3f& _center, const Vec3f& _halfSize) : center(_center), halfSize(_halfSize) {}

    Vec3f center   = {0.f, 0.f, 0.f};
    Vec3f halfSize = {0.f, 0.f, 0.f};

    Vec3f Min() const { return center - halfSize; }
    Vec3f Max() const { return center + halfSize; };
};
}
