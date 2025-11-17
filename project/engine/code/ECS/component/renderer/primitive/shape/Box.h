#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace Primitive {

constexpr int32_t kBoxVertexCount = 24;
constexpr int32_t kBoxIndexCount  = 36;

// <summary>
/// Box(立方体)のPrimitiveクラス
/// </summary>
struct Box
    : public IPrimitive {
public:
    Box(int32_t _vertexSize = kBoxVertexCount, int32_t _indexSize = kBoxIndexCount) : IPrimitive(PrimitiveType::Box) {
        vertexSize_ = _vertexSize; // 立方体の頂点数
        indexSize_  = _indexSize; // 立方体のインデックス数
    }
    ~Box() override {}
    void CreateMesh(TextureMesh* _mesh) override;

public:
    Vec3f halfSize_;
};

}
