#pragma once

/// enum
#include "GeometryType.h"

namespace geometry {

/// <summary>
/// 形状の基底クラス
/// </summary>
class IGeometry {
public:
    IGeometry(Type _type) : type_(_type) {}
    virtual ~IGeometry() {}

private:
    Type type_;

public:
    Type getType() const { return type_; }
};

} // namespace geometry
