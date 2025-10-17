#pragma once

#include "geometry/base/IGeometry.h"

/// math
#include "math/Vector2.h"
#include "math/Vector3.h"

namespace geometry {

/// <summary>
/// Plane(面)のGeometryクラス
/// </summary>
struct Plane
    : public IGeometry {
public:
    Plane() : IGeometry(Type::Plane) {}
    ~Plane() override {}

public:
    Vec2f size_   = {1.0f, 1.0f}; // 面のサイズ(半径ではない)
    Vec3f normal_ = {0.0f, 0.f, 1.0f}; // 面の法線ベクトル(※必ず Normalizeすること)
};

}
