#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace OriGine {
namespace Primitive {

/// <summary>
/// Plane(面)のPrimitiveクラス
/// </summary>
struct Plane
    : public IPrimitive {
public:
    Plane() : IPrimitive(PrimitiveType::Plane) {}
    ~Plane() override {}

    void CreateMesh(TextureColorMesh* _mesh) override;

public:
    Vec2f size_   = {1.0f, 1.0f}; // 平面の幅・高さ
    Vec2f uv_     = {1.0f, 1.0f}; // UV座標のスケール
    // 平面の法線方向。頂点は常にXY平面上(Z=0)に生成されるため、normal_を変えても
    // 頂点位置そのものは回転しない(法線属性のみが変わる)。実際に平面を傾けたい場合は
    // エンティティのTransformで回転させる必要がある
    Vec3f normal_ = {0.0f, 0.0f, 1.0f}; // 平面の法線方向
};

} // namespace Primitive
} // namespace OriGine
