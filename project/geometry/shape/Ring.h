#pragma once

#include "geometry/base/IGeometry.h"

namespace geometry {

/// <summary>
/// Ring(環)のGeometryクラス
/// </summary>
struct Ring
    : public IGeometry {
public:
    Ring() : IGeometry(Type::Ring) {}
    ~Ring() override {}

public:
    float innerRadius_ = 0.5f; ///< 内側の半径
    float outerRadius_ = 1.0f; ///< 外側の半径
};

} // namespace geometry
