#pragma once

/// enum
#include "PrimitiveType.h"
/// engine
// directX12
#include "directX12/mesh/Mesh.h"

namespace OriGine {
namespace Primitive {

/// <summary>
/// 形状の基底クラス
/// </summary>
class IPrimitive {
public:
    IPrimitive(PrimitiveType _type) : type_(_type) {}
    virtual ~IPrimitive() {}

    virtual void CreateMesh(TextureMesh* _mesh) = 0;

protected:
    int32_t vertexSize_ = 0; // 頂点数
    int32_t indexSize_  = 0; // インデックス数
private:
    PrimitiveType type_;

public: // accessor
    PrimitiveType GetType() const { return type_; }

    int32_t GetVertexSize() const {
        return vertexSize_;
    }
    void SetVertexSize(int32_t _size) {
        vertexSize_ = _size;
    }
    int32_t GetIndexSize() const {
        return indexSize_;
    }
    void SetIndexSize(int32_t _size) {
        indexSize_ = _size;
    }
};

template <typename T>
concept IsPrimitive = std::derived_from<T, IPrimitive>;
}
} // namespace OriGine
