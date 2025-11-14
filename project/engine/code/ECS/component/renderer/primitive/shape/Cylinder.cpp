#include "Cylinder.h"

namespace Primitive {
void Cylinder::CreateMesh(TextureMesh* _mesh) {
    if (_mesh->getIndexCapacity() < indexSize_) {
        // 必要なら Finalize
        if (_mesh->getVertexBuffer().getResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }

    float radPerDiv = kTao / float(kCylinderDivisions);

    std::vector<TextureMesh::VertexType> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(vertexSize_);
    indices.reserve(indexSize_);
    for (size_t i = 0; i < kCylinderDivisions; ++i) {
        float sin     = std::sin(radPerDiv * static_cast<float>(i));
        float cos     = std::cos(radPerDiv * static_cast<float>(i));
        float sinNext = std::sin(radPerDiv * static_cast<float>(i + 1));
        float cosNext = std::cos(radPerDiv * static_cast<float>(i + 1));
        float u       = static_cast<float>(i) / static_cast<float>(kCylinderDivisions);
        float uNext   = static_cast<float>(i + 1) / static_cast<float>(kCylinderDivisions);

        vertices.emplace_back(TextureVertexData(
            Vec4f(-sin * topRadius_[X], height_, cos * topRadius_[Y], 1.0f),
            Vec2f(u, 0.0f),
            Vec3f(-sin, 0.f, -cos)));
        vertices.emplace_back(TextureVertexData(
            Vec4f(-sinNext * topRadius_[X], height_, cosNext * topRadius_[Y], 1.0f),
            Vec2f(uNext, 0.0f),
            Vec3f(-sinNext, 0.f, -cosNext)));
        vertices.emplace_back(TextureVertexData(
            Vec4f(-sin * bottomRadius_[X], 0, cos * bottomRadius_[Y], 1.0f),
            Vec2f(u, 1.0f),
            Vec3f(-sin, 0.f, -cos)));
        vertices.emplace_back(TextureVertexData(
            Vec4f(-sin * bottomRadius_[X], 0, cos * bottomRadius_[Y], 1.0f),
            Vec2f(u, 1.0f),
            Vec3f(-sin, 0.f, -cos)));
        vertices.emplace_back(TextureVertexData(
            Vec4f(-sinNext * topRadius_[X], height_, cos * topRadius_[Y], 1.0f),
            Vec2f(uNext, 0.0f),
            Vec3f(-sin, 0.f, -cos)));
        vertices.emplace_back(TextureVertexData(
            Vec4f(-sinNext * bottomRadius_[X], height_, cos * bottomRadius_[Y], 1.0f),
            Vec2f(uNext, 1.0f),
            Vec3f(-sin, 0.f, -cos)));

        indices.push_back(static_cast<uint32_t>(i * 6 + 0));
        indices.push_back(static_cast<uint32_t>(i * 6 + 1));
        indices.push_back(static_cast<uint32_t>(i * 6 + 2));
        indices.push_back(static_cast<uint32_t>(i * 6 + 3));
        indices.push_back(static_cast<uint32_t>(i * 6 + 4));
        indices.push_back(static_cast<uint32_t>(i * 6 + 5));
    }

    _mesh->setVertexData(vertices);
    _mesh->setIndexData(indices);

    _mesh->TransferData();
}
} // namespace Primitive
