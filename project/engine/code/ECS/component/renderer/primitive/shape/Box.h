#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace Primitive {

// <summary>
/// Box(立方体)のPrimitiveクラス
/// </summary>
class Box
    : public IPrimitive {
public:
    Box(int32_t _vertexSize = 8, int32_t _indexSize = 36) : IPrimitive(PrimitiveType::Box) {
        vertexSize_ = _vertexSize; // 立方体の頂点数
        indexSize_  = _indexSize; // 立方体のインデックス数
    }
    ~Box() override {}
    void createMesh(TextureMesh* _mesh) override;

private:
    Vec3f halfSize_;

public:
    const Vec3f& getHalf() const {
        return halfSize_;
    }
    void setHalfSize(const Vec3f& _halfSize) {
        halfSize_ = _halfSize;
    }

    Vec3f getSize() const {
        return halfSize_ * 2.0f;
    }
    void setSize(const Vec3f& _size) {
        halfSize_ = _size / 2.0f;
    }
};

}
