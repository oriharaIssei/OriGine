#include "Ring.h"

/// math
#include "math/mathEnv.h"

namespace Primitive {

/// =====================================================
/// Ring
/// =====================================================
void Ring::createMesh(TextureMesh* _mesh) {
    if (!_mesh->vertexes_.empty()) {
        _mesh->vertexes_.clear();
    }
    if (!_mesh->indexes_.empty()) {
        _mesh->indexes_.clear();
    }

    const float realDivision    = static_cast<float>(division_);
    const float radianPerDivide = kTao / realDivision;

    vertexSize_ = division_ * 4; // 1つの円環は division_ * 4 頂点
    indexSize_  = division_ * 6; // 1つの円環は division_ * 6 インデックス

    if (_mesh->getIndexCapacity() < (int32_t)indexSize_) {
        // 必要なら Finalize
        if (_mesh->getVertexBuffer().getResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }

    // 円環の頂点を計算
    for (uint32_t i = 0; i < division_; ++i) {
        float realIndex = static_cast<float>(i);
        float nextIndex = static_cast<float>(i + 1);

        float sin     = std::sin(radianPerDivide * realIndex);
        float cos     = std::cos(radianPerDivide * realIndex);
        float sinNext = std::sin(radianPerDivide * nextIndex);
        float cosNext = std::cos(radianPerDivide * nextIndex);
        float u       = realIndex / realDivision;
        float uNext   = nextIndex / realDivision;

        // Vertex
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sin * outerRadius_, cos * outerRadius_, 0.f, 1.f), Vec2f(u, 0.f), Vector3f(0.f, 0.f, -1.f)));
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sinNext * outerRadius_, cosNext * outerRadius_, 0.f, 1.f), Vec2f(uNext, 0.f), Vector3f(0.f, 0.f, -1.f)));
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sin * innerRadius_, cos * innerRadius_, 0.f, 1.f), Vec2f(u, 1.f), Vector3f(0.f, 0.f, -1.f)));
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sinNext * innerRadius_, cosNext * innerRadius_, 0.f, 1.f), Vec2f(uNext, 1.f), Vector3f(0.f, 0.f, -1.f)));

        // Index
        uint32_t startIndex = i * 4;
        _mesh->indexes_.emplace_back(startIndex);
        _mesh->indexes_.emplace_back(startIndex + 2);
        _mesh->indexes_.emplace_back(startIndex + 1);
        _mesh->indexes_.emplace_back(startIndex + 1);
        _mesh->indexes_.emplace_back(startIndex + 2);
        _mesh->indexes_.emplace_back(startIndex + 3);
    }

    _mesh->TransferData();
}

}
