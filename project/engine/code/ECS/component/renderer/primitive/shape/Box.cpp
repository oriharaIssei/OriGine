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
    // インデックスデータを設定
    std::vector<uint32_t> indices;
    indices.reserve(indexSize_);

    Vec3f minPos = -halfSize_;
    Vec3f maxPos = halfSize_;

    // 面法線
    const Vec3f normals[6] = {
        Vec3f(0, 0, -1), // front (手前)
        Vec3f(0, 0, 1), // back  (奥)
        Vec3f(1, 0, 0), // right
        Vec3f(-1, 0, 0), // left
        Vec3f(0, 1, 0), // top
        Vec3f(0, -1, 0) // bottom
    };

    auto addFace = [&](int normalIndex,
                       const Vec3f& p0, const Vec3f& p1,
                       const Vec3f& p2, const Vec3f& p3) {
        const Vec3f& n = normals[normalIndex];
        uint32_t base  = static_cast<uint32_t>(vertices.size());

        // p0-p1-p2-p3 は必ず CCW にする
        vertices.emplace_back(TextureVertexData(Vec4f(p0, 1), Vec2f(0.f, 0.f), n));
        vertices.emplace_back(TextureVertexData(Vec4f(p1, 1), Vec2f(1.f, 0.f), n));
        vertices.emplace_back(TextureVertexData(Vec4f(p2, 1), Vec2f(0.f, 1.f), n));
        vertices.emplace_back(TextureVertexData(Vec4f(p3, 1), Vec2f(1.f, 1.f), n));

        indices.insert(indices.end(),
            {base + 0, base + 1, base + 2,
                base + 2, base + 1, base + 3});
    };

    // front (-Z)
    addFace(0,
        Vec3f(minPos[X], minPos[Y], minPos[Z]),
        Vec3f(minPos[X], maxPos[Y], minPos[Z]),
        Vec3f(maxPos[X], minPos[Y], minPos[Z]),
        Vec3f(maxPos[X], maxPos[Y], minPos[Z]));

    // back (+Z)
    addFace(1,
        Vec3f(minPos[X], minPos[Y], maxPos[Z]),
        Vec3f(minPos[X], maxPos[Y], maxPos[Z]),
        Vec3f(maxPos[X], minPos[Y], maxPos[Z]),
        Vec3f(maxPos[X], maxPos[Y], maxPos[Z]));

    // right (+X)
    addFace(2,
        Vec3f(maxPos[X], minPos[Y], minPos[Z]),
        Vec3f(maxPos[X], minPos[Y], maxPos[Z]),
        Vec3f(maxPos[X], maxPos[Y], minPos[Z]),
        Vec3f(maxPos[X], maxPos[Y], maxPos[Z]));

    // left (-X)
    addFace(3,
        Vec3f(minPos[X], minPos[Y], minPos[Z]),
        Vec3f(minPos[X], minPos[Y], maxPos[Z]),
        Vec3f(minPos[X], maxPos[Y], minPos[Z]),
        Vec3f(minPos[X], maxPos[Y], maxPos[Z]));

    // top (+Y)
    addFace(4,
        Vec3f(minPos[X], maxPos[Y], minPos[Z]),
        Vec3f(minPos[X], maxPos[Y], maxPos[Z]),
        Vec3f(maxPos[X], maxPos[Y], minPos[Z]),
        Vec3f(maxPos[X], maxPos[Y], maxPos[Z]));

    // bottom (-Y)
    addFace(5,
        Vec3f(minPos[X], minPos[Y], minPos[Z]),
        Vec3f(maxPos[X], minPos[Y], minPos[Z]),
        Vec3f(minPos[X], minPos[Y], maxPos[Z]),
        Vec3f(maxPos[X], minPos[Y], maxPos[Z]));

    _mesh->setVertexData(vertices);
    _mesh->setIndexData(indices);

    _mesh->TransferData();
}
}
