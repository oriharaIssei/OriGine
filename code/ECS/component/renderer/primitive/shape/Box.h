#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace OriGine {
namespace Primitive {

constexpr int32_t kBoxVertexCount = 24; // Boxのデフォルト頂点数(各面4頂点 x 6面)
constexpr int32_t kBoxIndexCount  = 36; // Boxのデフォルトインデックス数(三角形2枚 x 3頂点 x 6面)

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
    void CreateMesh(TextureColorMesh* _mesh) override;

public:
    Vec3f halfSize_; // 中心から各軸方向への半径(半辺長)
};
}
} // namespace OriGine
