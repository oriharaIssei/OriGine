#include "Plane.h"

namespace Primitive {

/// =====================================================
/// Plane
/// =====================================================
void Plane::CreateMesh(TextureMesh* _mesh) {
    vertexSize_ = 4; // 頂点数
    indexSize_  = 6; // インデックス数

    if ((int32_t)_mesh->getIndexCapacity() < indexSize_) {
        // 必要なら Finalize
        if (_mesh->getVertexBuffer().getResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }

    // XZ 平面を作成
    _mesh->setVertexData({{Vec4f(-size_[X], size_[Y], 0.0f, 1.0f), Vec2f(0.0f, 0.0f), normal_},
        {Vec4f(size_[X], size_[Y], 0.0f, 1.0f), Vec2f(uv_[X], 0.0f), normal_},
        {Vec4f(-size_[X], -size_[Y], 0.0f, 1.0f), Vec2f(0.0f, uv_[Y]), normal_},
        {Vec4f(size_[X], -size_[Y], 0.0f, 1.0f), Vec2f(uv_[X], uv_[Y]), normal_}});
    _mesh->setIndexData(
        {0, 1, 2,
            1, 3, 2});
    _mesh->TransferData();
}

} // namespace Primitive
