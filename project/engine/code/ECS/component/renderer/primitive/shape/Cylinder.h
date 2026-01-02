#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

/// math
#include "math/mathEnv.h"
#include "math/MyEasing.h"
#include <cstdint>

namespace OriGine {
namespace Primitive {

constexpr uint32_t kCylinderRadialDivisions = 36; // 円柱の分割数
constexpr uint32_t kCylinderHeightDivisons  = 8; // 高さ方向の分割数

struct Cylinder
    : public IPrimitive {
public:
    Cylinder(uint32_t _radialDivisions = kCylinderRadialDivisions, uint32_t _heightDivisions = kCylinderHeightDivisons)
        : IPrimitive(PrimitiveType::Cylinder) {
        radialDivisions = _radialDivisions;
        heightDivisions = _heightDivisions;

        // 頂点数とインデックス数の設定
        vertexSize_ = (radialDivisions + 1) * (heightDivisions + 1);
        indexSize_  = radialDivisions * heightDivisions * 6 + radialDivisions * 6;
    }

    ~Cylinder() override {}

    void CreateMesh(TextureColorMesh* _mesh) override;

public:
    Vec2f topRadius    = Vec2f(1.f, 1.f); // 上面の半径
    Vec2f bottomRadius = Vec2f(1.f, 1.f); // 底面の半径
    float height       = 1.f; // 高さ

    uint32_t radialDivisions = kCylinderRadialDivisions;
    uint32_t heightDivisions = kCylinderHeightDivisons;

    EaseType radiusEaseType = EaseType::Liner; // 半径のイージングタイプ
};
}
} // namespace OriGine
