#pragma once

#include "base/IBounds.h"

/// math
#include "Vector3.h"

namespace math::bounds {
struct AABB
    : public IBounds {
    AABB() {}
    AABB(const Vec3f& _min, const Vec3f& _max) : min_(_min), max_(_max) {}

    Vec3f min_ = {0.f, 0.f, 0.f};
    Vec3f max_ = {0.f, 0.f, 0.f};
};

}
