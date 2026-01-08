#include "Cylinder.h"

namespace OriGine {
namespace Primitive {
void Cylinder::CreateMesh(TextureColorMesh* _mesh) {

    // 頂点数とインデックス数の設定
    vertexSize_ = (radialDivisions + 1) * (heightDivisions + 1);
    indexSize_  = radialDivisions * heightDivisions * 6 + radialDivisions * 6;

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
    float angleStep = kTau / float(radialDivisions);

    std::vector<TextureColorMesh::VertexType> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(vertexSize_);
    indices.reserve(indexSize_);

    // ==============================
    // 頂点生成
    // ==============================
    int32_t radiusEaseTypeInt = static_cast<int32_t>(radiusEaseType);
    for (uint32_t h = 0; h <= heightDivisions; ++h) {

        float v      = float(h) / float(heightDivisions);
        float easedV = EasingFunctions[radiusEaseTypeInt](v);

        Vec2f radius = Lerp(bottomRadius, topRadius, easedV);
        float y      = std::lerp(0.0f, height, v);

        for (uint32_t r = 0; r <= radialDivisions; ++r) {

            float angle = angleStep * float(r);
            float s     = sinf(angle);
            float c     = cosf(angle);

            Vec3f pos(s * radius[X], y, c * radius[Y]);

            float u = float(r) / float(radialDivisions);

            TextureColorMesh::VertexType vertex{};
            vertex.pos      = Vec4f(pos, 1.f);
            vertex.normal   = Vec3f(s, 0.0f, c); // 側面法線
            vertex.texCoord = Vec2f(u, v);

            vertices.emplace_back(vertex);
        }
    }

    // ==============================
    // インデックス生成
    // ==============================
    uint32_t ringVertexCount = radialDivisions + 1;

    for (uint32_t h = 0; h < heightDivisions; ++h) {
        for (uint32_t r = 0; r < radialDivisions; ++r) {

            uint32_t i0 = h * ringVertexCount + r;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + ringVertexCount;
            uint32_t i3 = i2 + 1;

            // triangle 1
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            // triangle 2
            indices.push_back(i1);
            indices.push_back(i3);
            indices.push_back(i2);
        }
    }

    _mesh->SetVertexData(vertices);
    _mesh->SetIndexData(indices);

    _mesh->TransferData();
}

} // namespace Primitive
} // namespace OriGine
