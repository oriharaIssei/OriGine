#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

/// math
#include "math/mathEnv.h"

namespace Primitive {
constexpr uint32_t kCylinderDivisions   = 36; // 円柱の分割数
constexpr uint32_t kCylinderVertexCount = kCylinderDivisions + 1 + kCylinderDivisions + 1; // 円柱の頂点数
constexpr uint32_t kCylinderIndexCount  = kCylinderDivisions * 3 * 2 + kCylinderDivisions * 3; // 円柱のインデックス数

struct Cylinder
    : public IPrimitive {
public:
    Cylinder(int32_t _vertexSize = 36 + 1 + 36 + 1, int32_t _indexSize = 36 * 3 * 2 + 36 * 3)
        : IPrimitive(PrimitiveType::Cylinder) {
        vertexSize_ = _vertexSize; // 円柱の頂点数
        indexSize_  = _indexSize; // 円柱のインデックス数
    }
    ~Cylinder() override {}

    void CreateMesh(TextureMesh* _mesh) override;

public:
    Vec2f topRadius_    = Vec2f(1.f, 1.f); // 上面の半径
    Vec2f bottomRadius_ = Vec2f(1.f, 1.f); // 底面の半径
    float height_       = 1.f; // 高さ
};

}
