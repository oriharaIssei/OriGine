#include "Ring.h"

/// math
#include "math/MathEnv.h"

namespace OriGine {
namespace Primitive {

/// =====================================================
/// Ring
/// =====================================================
void Ring::CreateMesh(TextureColorMesh* _mesh) {
    if (!_mesh->vertexes_.empty()) {
        _mesh->vertexes_.clear();
    }
    if (!_mesh->indexes_.empty()) {
        _mesh->indexes_.clear();
    }

    const float realDivision    = static_cast<float>(division_);
    const float radianPerDivide = kTau / realDivision; // kTau(2pi)を分割数で割った、1区画あたりの中心角

    // Sphere/Cylinderのようにリングを共有せず、区画(扇形の台形)ごとに外周2点・内周2点の
    // 計4頂点を都度生成する。頂点を隣の区画と共有しないぶん頂点数は増えるが、
    // シームのつなぎ目を気にせず単純なループで生成できる
    vertexSize_ = division_ * 4; // 1つの円環は division_ * 4 頂点
    indexSize_  = division_ * 6; // 1区画=2枚の三角形(3頂点 x 2) なので division_ * 6 インデックス

    if (_mesh->GetIndexCapacity() < (int32_t)indexSize_) {
        // 必要なら Finalize
        if (_mesh->GetVertexBuffer().GetResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }

    // 円環の頂点を計算
    // 各区画iについて、開始角(realIndex)と終了角(nextIndex)の2方向 x 外周・内周の2半径分、
    // 計4頂点(外周始点・外周終点・内周始点・内周終点)を生成しXY平面上に配置する(法線は常に-Z、正面向き)
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
        _mesh->vertexes_.emplace_back(TextureColorVertexData(Vector4f(-sin * outerRadius_, cos * outerRadius_, 0.f, 1.f), Vec2f(u, 0.f), Vector3f(0.f, 0.f, -1.f), kWhite));
        _mesh->vertexes_.emplace_back(TextureColorVertexData(Vector4f(-sinNext * outerRadius_, cosNext * outerRadius_, 0.f, 1.f), Vec2f(uNext, 0.f), Vector3f(0.f, 0.f, -1.f), kWhite));
        _mesh->vertexes_.emplace_back(TextureColorVertexData(Vector4f(-sin * innerRadius_, cos * innerRadius_, 0.f, 1.f), Vec2f(u, 1.f), Vector3f(0.f, 0.f, -1.f), kWhite));
        _mesh->vertexes_.emplace_back(TextureColorVertexData(Vector4f(-sinNext * innerRadius_, cosNext * innerRadius_, 0.f, 1.f), Vec2f(uNext, 1.f), Vector3f(0.f, 0.f, -1.f), kWhite));

        // Index: 4頂点(外周始点0, 外周終点1, 内周始点2, 内周終点3)を対角線0-3ではなく
        // 1-2で2枚の三角形に分割し、いずれも-Z方向(法線方向)から見てCCWになるよう巻く
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
}
