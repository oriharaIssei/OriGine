#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace Primitive {

/// <summary>
/// Ring(環)のPrimitiveクラス
/// </summary>
struct Ring
    : public IPrimitive {
public:
    Ring() : IPrimitive(PrimitiveType::Ring) {}
    ~Ring() override {}

    void CreateMesh(TextureMesh* _mesh) override;

public:
    uint32_t division_ = 16;

    float outerRadius_ = 1.f;
    float innerRadius_ = 0.5f;
};

} // namespace Primitive
