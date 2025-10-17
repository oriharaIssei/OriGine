#pragma once

#include "geometry/base/IGeometry.h"

namespace geometry {

/// <summary>
/// Sphere(球)のGeometryクラス
/// </summary>
struct Sphere
    : public IGeometry {
public:
    Sphere() : IGeometry(Type::Sphere) {}
    ~Sphere() override {}

public:
    float radius_ = 1.0f; ///< 球の半径
};
}
