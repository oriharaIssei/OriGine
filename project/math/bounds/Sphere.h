#pragma once

#include "base/IBounds.h"

/// math
#include "Vector3.h"

namespace math::bounds {

struct Sphere
    : public IBounds {
    Sphere() {}
    Sphere(const Vec3f& _center, float _radius) : center_(_center), radius_(_radius) {}

    Vec3f center_ = {0.f, 0.f, 0.f};
    float radius_ = 0.f;
};

}
