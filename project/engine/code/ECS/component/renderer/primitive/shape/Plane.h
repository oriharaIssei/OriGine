#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace Primitive {

/// <summary>
/// Plane(面)のPrimitiveクラス
/// </summary>
struct Plane
    : public IPrimitive {
public:
    Plane() : IPrimitive(PrimitiveType::Plane) {}
    ~Plane() override {}

    void CreateMesh(TextureMesh* _mesh) override;

public:
    Vec2f size_   = {1.0f, 1.0f};
    Vec2f uv_     = {1.0f, 1.0f};
    Vec3f normal_ = {0.0f, 0.0f, 1.0f};
};
}
