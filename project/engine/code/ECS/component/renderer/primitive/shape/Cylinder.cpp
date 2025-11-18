#include "Cylinder.h"

namespace Primitive {
void Cylinder::CreateMesh(TextureMesh* _mesh) {
    if (_mesh->GetIndexCapacity() < indexSize_) {
        // 必要なら Finalize
        if (_mesh->GetVertexBuffer().GetResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }

    // 角度ステップ
    float angleStep = kTao / float(kCylinderDivisions);

    std::vector<TextureMesh::VertexType> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(vertexSize_);
    indices.reserve(indexSize_);

    // cylinderを生成
    // 頂点
    for (uint32_t i = 0; i <= kCylinderDivisions; ++i) {

        // 360度を等間隔で分割したi番目の角度
        float angle = angleStep * (float)i;
        float sin   = sinf(angle);
        float cos   = cosf(angle);

        // 上面の頂点位置
        Vec3f topVertex = Vec3f(sin * topRadius_[X], height_, cos * topRadius_[Y]);
        // 下面の頂点位置
        Vec3f bottomVertex = Vec3f(sin * bottomRadius_[X], 0.0f, cos * bottomRadius_[Y]);

        float u = static_cast<float>(i) / static_cast<float>(kCylinderDivisions);

        TextureMesh::VertexType vertex{};
        vertex.pos      = Vec4f(topVertex, 1.f);
        vertex.texCoord = Vec2f(-u, 0.0f);
        vertices.emplace_back(vertex);

        vertex.pos      = Vec4f(bottomVertex, 1.f);
        vertex.texCoord = Vec2f(-u, 1.0f);
        vertices.emplace_back(vertex);
    }

    // インデックス
    for (uint32_t i = 0; i < kCylinderDivisions; ++i) {

        uint32_t t0 = i * 2;
        uint32_t b0 = i * 2 + 1;
        uint32_t t1 = (i + 1) * 2;
        uint32_t b1 = (i + 1) * 2 + 1;

        indices.push_back(t0);
        indices.push_back(t1);
        indices.push_back(b0);

        indices.push_back(t1);
        indices.push_back(b1);
        indices.push_back(b0);
    }

    _mesh->SetVertexData(vertices);
    _mesh->SetIndexData(indices);

    _mesh->TransferData();
}
} // namespace Primitive
