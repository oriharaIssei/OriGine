#pragma once

#include "base/IBounds.h"

/// math
#include "Vector3.h"

namespace OriGine {
namespace Bounds {

/// <summary>
/// Sphere
/// </summary>
struct Sphere
    : public IBounds {
    Sphere() {}
    Sphere(const Vec3f& center, float radius) : center_(center), radius_(radius) {}

    Vec3f center_ = {0.f, 0.f, 0.f};
    float radius_ = 0.f;
};

} // namespace Bounds
} // namespace OriGine
