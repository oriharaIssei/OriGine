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
    Vec3f normal_ = {0.0f, 0.0f, 1.0f}; // 平面の法線方向
};

} // namespace Primitive
} // namespace OriGine
