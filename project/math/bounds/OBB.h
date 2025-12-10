#pragma once

#include "base/IBounds.h"

/// math
#include "Orientation.h"
#include "Vector3.h"

namespace OriGine {
namespace Bounds {

/// <summary>
/// OBB (Oriented Bounding Box)
/// </summary>
struct OBB
    : public IBounds {
    OBB() {}
    OBB(const Vec3f& _center, const Vec3f& _halfSize, const Orientation& _orientations)
        : center_(_center), halfSize_(_halfSize), orientations_(_orientations) {}

    Vec3f center_             = {0.f, 0.f, 0.f};
    Vec3f halfSize_           = {0.f, 0.f, 0.f};
    Orientation orientations_ = Orientation::Identity();
};

} // namespace Bounds
} // namespace OriGine
