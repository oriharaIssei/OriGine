#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

/// math
#include "math/MathEnv.h"
#include "math/MyEasing.h"
#include <cstdint>

namespace OriGine {
namespace Primitive {

constexpr uint32_t kCylinderRadialDivisions = 36; // 円柱の分割数 (10度刻みで一周する滑らかさの目安値)
constexpr uint32_t kCylinderHeightDivisions = 8; // 高さ方向の分割数

/// <summary>
/// Cylinder(円柱)のPrimitiveクラス
/// </summary>
struct Cylinder
    : public IPrimitive {
public:
    Cylinder(uint32_t _radialDivisions = kCylinderRadialDivisions, uint32_t _heightDivisions = kCylinderHeightDivisions)
        : IPrimitive(PrimitiveType::Cylinder) {
        radialDivisions = _radialDivisions;
        heightDivisions = _heightDivisions;

        // 頂点数とインデックス数の設定
        // (円周方向・高さ方向とも、UVの継ぎ目を成立させるためリング/段を+1して確保する。
        //  詳細な算出根拠はCylinder::CreateMeshのコメントを参照)
        vertexSize_ = (radialDivisions + 1) * (heightDivisions + 1);
        indexSize_  = radialDivisions * heightDivisions * 6 + radialDivisions * 6;
    }

    ~Cylinder() override {}

    void CreateMesh(TextureColorMesh* _mesh) override;

public:
    Vec2f topRadius    = Vec2f(1.f, 1.f); // 上面の半径
    Vec2f bottomRadius = Vec2f(1.f, 1.f); // 底面の半径
    float height       = 1.f; // 高さ

    uint32_t radialDivisions = kCylinderRadialDivisions; // 円周方向の分割数
    uint32_t heightDivisions = kCylinderHeightDivisions; // 高さ方向の分割数

    EaseType radiusEaseType = EaseType::Linear; // 半径のイージングタイプ
};
}
} // namespace OriGine
