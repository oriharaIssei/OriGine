#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace OriGine {
namespace Primitive {

/// <summary>
/// Ring(環)のPrimitiveクラス
/// </summary>
struct Ring
    : public IPrimitive {
public:
    Ring() : IPrimitive(PrimitiveType::Ring) {}
    ~Ring() override {}

    void CreateMesh(TextureColorMesh* _mesh) override;

public:
    uint32_t division_ = 16; // 円周方向の分割数

    float outerRadius_ = 1.f; // 外径
    float innerRadius_ = 0.5f; // 内径
};

} // namespace Primitive
} // namespace OriGine
