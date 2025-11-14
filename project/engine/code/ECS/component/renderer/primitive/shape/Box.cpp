#include "Box.h"

namespace Primitive {

/// =====================================================
/// Box
/// =====================================================
void Box::CreateMesh(TextureMesh* _mesh) {
    if (_mesh->getIndexCapacity() < indexSize_) {
        // 必要なら Finalize
        if (_mesh->getVertexBuffer().getResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }

    // 頂点データを設定
    std::vector<TextureMesh::VertexType> vertices;
    vertices.reserve(vertexSize_);

    Vec3f minPos   = -halfSize_;
    Vec3f maxPos   = halfSize_;

    // 立方体の8つの頂点を定義
    // left bottom front
    vertices.emplace_back(TextureVertexData(Vec4f(minPos[X], minPos[Y], minPos[Z], 1.0f), Vec2f(0.0f, 0.0f), Vec3f(-1.0f, -1.0f, -1.0f)));
    // right bottom front
    vertices.emplace_back(TextureVertexData(Vec4f(maxPos[X], minPos[Y], minPos[Z], 1.0f), Vec2f(1.0f, 0.0f), Vec3f(1.0f, -1.0f, -1.0f)));
    // left top front
    vertices.emplace_back(TextureVertexData(Vec4f(minPos[X], maxPos[Y], minPos[Z], 1.0f), Vec2f(0.0f, 1.0f), Vec3f(-1.0f, 1.0f, -1.0f)));
    // right top front
    vertices.emplace_back(TextureVertexData(Vec4f(maxPos[X], maxPos[Y], minPos[Z], 1.0f), Vec2f(1.0f, 1.0f), Vec3f(1.0f, 1.0f, -1.0f)));
    // left bottom back
    vertices.emplace_back(TextureVertexData(Vec4f(minPos[X], minPos[Y], maxPos[Z], 1.0f), Vec2f(0.0f, 0.0f), Vec3f(-1.0f, -1.0f, 1.0f)));
    // right bottom back
    vertices.emplace_back(TextureVertexData(Vec4f(maxPos[X], minPos[Y], maxPos[Z], 1.0f), Vec2f(1.0f, 0.0f), Vec3f(1.0f, -1.0f, 1.0f)));
    // left top back
    vertices.emplace_back(TextureVertexData(Vec4f(minPos[X], maxPos[Y], maxPos[Z], 1.0f), Vec2f(0.0f, 1.0f), Vec3f(-1.0f, 1.0f, 1.0f)));
    // right top back
    vertices.emplace_back(TextureVertexData(Vec4f(maxPos[X], maxPos[Y], maxPos[Z], 1.0f), Vec2f(1.0f, 1.0f), Vec3f(1.0f, 1.0f, 1.0f)));
    _mesh->setVertexData(vertices);

    // インデックスデータを設定
    std::vector<uint32_t> indices;
    indices.reserve(indexSize_);

    // 前面 (z = min) : 外向き -Z
    indices.insert(indices.end(), {0, 2, 1, 1, 2, 3});
    // 背面 (z = max) : 外向き +Z
    indices.insert(indices.end(), {4, 5, 6, 5, 7, 6});
    // 左面 (x = min) : 外向き -X
    indices.insert(indices.end(), {0, 4, 2, 2, 4, 6});
    // 右面 (x = max) : 外向き +X
    indices.insert(indices.end(), {1, 3, 5, 3, 7, 5});
    // 上面 (y = max) : 外向き +Y
    indices.insert(indices.end(), {2, 6, 3, 3, 6, 7});
    // 下面 (y = min) : 外向き -Y
    indices.insert(indices.end(), {0, 1, 4, 1, 5, 4});

    _mesh->setIndexData(indices);

    _mesh->TransferData();
}

}
