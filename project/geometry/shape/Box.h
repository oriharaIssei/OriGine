#pragma once

#include "geometry/base/IGeometry.h"

/// math
#include "math/Vector3.h"

namespace geometry {

// <summary>
/// Box(立方体)のGeometryクラス
/// </summary>
struct Box
    : public IGeometry {
public:
    Box() : IGeometry(Type::Box) {}
    ~Box() override {}

public:
    Vec3f halfSize_ = Vec3f(1.0f, 1.0f, 1.0f); ///< 立方体の半分のサイズ
};

} // namespace geometry
