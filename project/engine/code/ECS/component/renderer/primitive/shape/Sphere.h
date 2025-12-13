#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace OriGine {
namespace Primitive {

/// <summary>
/// Sphere(球)のPrimitiveクラス
/// </summary>
struct Sphere
    : public IPrimitive {
public:
    Sphere() : IPrimitive(PrimitiveType::Sphere) {}
    ~Sphere() override {}
    void CreateMesh(TextureColorMesh* _mesh) override;

public:
    float radius_               = 1.0f; // 半径
    uint32_t divisionLatitude_  = 8; // 緯度分割数
    uint32_t divisionLongitude_ = 16; // 経度分割数
};

} // namespace Primitive
} // namespace OriGine
