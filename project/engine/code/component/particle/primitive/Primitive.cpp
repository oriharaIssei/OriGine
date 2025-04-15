#include "Primitive.h"

/// =====================================================
/// Plane
/// =====================================================
void Plane::createMesh() {
    // XZ 平面を作成
    mesh_->setVertexData({{Vec4f(-size_[X], 0.0f, -size_[Y], 1.0f), Vec2f(0.0f, 0.0f), normal_},
        {Vec4f(size_[X], 0.0f, -size_[Y], 1.0f), Vec2f(uv_[X], 0.0f), normal_},
        {Vec4f(-size_[X], 0.0f, size_[Y], 1.0f), Vec2f(0.0f, uv_[Y]), normal_},
        {Vec4f(size_[X], 0.0f, size_[Y], 1.0f), Vec2f(uv_[X], uv_[Y]), normal_}});
    mesh_->setIndexData(
        {0, 1, 2,
            1, 3, 2});
    mesh_->TransferData();
}

/// =====================================================
/// Circle
/// =====================================================
void Circle::createMesh() {
    mesh_->vertexes_.clear();
    mesh_->indexes_.clear();
    // 円の頂点を計算
    for (uint32_t i = 0; i < vertexSize_; ++i) {
        float angle    = static_cast<float>(i) * 2.0f * std::numbers::pi_v<float> / static_cast<float>(vertexSize_);
        Vector3f point = {radius_ * std::cos(angle), 0.f, radius_ * std::sin(angle)};
        mesh_->vertexes_.emplace_back(TextureVertexData(Vec4f(point, 1.f), Vec2f(), point.normalize()));
    }
    // 円のインデックスを計算
    for (uint32_t i = 0; i < indexSize_; ++i) {
        mesh_->indexes_.emplace_back(i);
    }
    mesh_->TransferData();
}

/// =====================================================
/// Ring
/// =====================================================
void Ring::createMesh() {
    mesh_->vertexes_.clear();
    mesh_->indexes_.clear();

    const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(division_);
    // 円環の頂点を計算
    for (uint32_t i = 0; i < division_; ++i) {
        float sin     = std::sin(radianPerDivide * static_cast<float>(i));
        float cos     = std::cos(radianPerDivide * static_cast<float>(i));
        float sinNext = std::sin(radianPerDivide * static_cast<float>(i + 1));
        float cosNext = std::cos(radianPerDivide * static_cast<float>(i + 1));
        float u       = static_cast<float>(i) / static_cast<float>(division_);
        float uNext   = static_cast<float>(i + 1) / static_cast<float>(division_);

        // Vertex
        mesh_->vertexes_.emplace_back(TextureVertexData(Vector4f(-sin * outerRadius_, cos * outerRadius_, 0.f, 1.f), Vec2f(u, 0.f), Vector3f(0.f, 0.f, -1.f)));
        mesh_->vertexes_.emplace_back(TextureVertexData(Vector4f(-sinNext * outerRadius_, cosNext * outerRadius_, 0.f, 1.f), Vec2f(uNext, 0.f), Vector3f(0.f, 0.f, -1.f)));
        mesh_->vertexes_.emplace_back(TextureVertexData(Vector4f(-sin * innerRadius_, cos * innerRadius_, 0.f, 1.f), Vec2f(u, 1.f), Vector3f(0.f, 0.f, -1.f)));
        mesh_->vertexes_.emplace_back(TextureVertexData(Vector4f(-sinNext * innerRadius_, cosNext * innerRadius_, 0.f, 1.f), Vec2f(uNext, 1.f), Vector3f(0.f, 0.f, -1.f)));

        // Index
        uint32_t startIndex = i * 4;
        mesh_->indexes_.emplace_back(startIndex);
        mesh_->indexes_.emplace_back(startIndex + 1);
        mesh_->indexes_.emplace_back(startIndex + 2);
        mesh_->indexes_.emplace_back(startIndex + 1);
        mesh_->indexes_.emplace_back(startIndex + 3);
        mesh_->indexes_.emplace_back(startIndex + 2);
    }

    mesh_->TransferData();
}

/// =====================================================
/// Sphere
/// =====================================================
void Sphere::createMesh() {
    mesh_->vertexes_.clear();
    mesh_->indexes_.clear();

    auto& vertexData = mesh_->vertexes_;
    auto& indexData  = mesh_->indexes_;

    float divisionReal    = static_cast<float>(division_);
    const float kLatEvery = std::numbers::pi_v<float> / divisionReal; // 緯度
    const float kLonEvery = 2.0f * std::numbers::pi_v<float> / divisionReal; // 経度

    uint32_t startVertexIndex = 0;
    uint32_t startIndexIndex  = 0;

    for (uint32_t latIndex = 0; latIndex < division_; ++latIndex) {
        float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; // theta
        // 経度方向に分割
        for (uint32_t lonIndex = 0; lonIndex < division_; ++lonIndex) {
            startVertexIndex = (latIndex * division_ + lonIndex) * 4;
            float lon        = lonIndex * kLonEvery; // fai
            // 頂点データを入力
            // a 左下
            vertexData[startVertexIndex].pos[0]      = std::cos(lat) * std::cos(lon);
            vertexData[startVertexIndex].pos[1]      = std::sin(lat);
            vertexData[startVertexIndex].pos[2]      = std::cos(lat) * std::sin(lon);
            vertexData[startVertexIndex].pos[3]      = 1.0f;
            vertexData[startVertexIndex].texCoord[0] = float(lonIndex) / divisionReal;
            vertexData[startVertexIndex].texCoord[1] = 1.0f - float(latIndex) / divisionReal;
            vertexData[startVertexIndex].normal      = {vertexData[startVertexIndex].pos[X], vertexData[startVertexIndex].pos[Y], vertexData[startVertexIndex].pos[Z]};

            // b 左上
            vertexData[startVertexIndex + 1].pos[0]      = std::cos(lat + kLatEvery) * std::cos(lon);
            vertexData[startVertexIndex + 1].pos[1]      = std::sin(lat + kLatEvery);
            vertexData[startVertexIndex + 1].pos[2]      = std::cos(lat + kLatEvery) * std::sin(lon);
            vertexData[startVertexIndex + 1].pos[3]      = 1.0f;
            vertexData[startVertexIndex + 1].texCoord[0] = float(lonIndex) / divisionReal;
            vertexData[startVertexIndex + 1].texCoord[1] = 1.0f - float(latIndex + 1) / divisionReal;
            vertexData[startVertexIndex + 1].normal      = {vertexData[startVertexIndex + 1].pos[X], vertexData[startVertexIndex + 1].pos[Y], vertexData[startVertexIndex + 1].pos[Z]};

            // c 右下
            vertexData[startVertexIndex + 2].pos[X]      = std::cos(lat) * std::cos(lon + kLonEvery);
            vertexData[startVertexIndex + 2].pos[Y]      = std::sin(lat);
            vertexData[startVertexIndex + 2].pos[Z]      = std::cos(lat) * std::sin(lon + kLonEvery);
            vertexData[startVertexIndex + 2].pos[W]      = 1.0f;
            vertexData[startVertexIndex + 2].texCoord[X] = float(lonIndex + 1) / divisionReal;
            vertexData[startVertexIndex + 2].texCoord[Y] = 1.0f - float(latIndex) / divisionReal;
            vertexData[startVertexIndex + 2].normal      = {vertexData[startVertexIndex + 2].pos[X], vertexData[startVertexIndex + 2].pos[Y], vertexData[startVertexIndex + 2].pos[Z]};

            // d 右上
            vertexData[startVertexIndex + 3].pos[X]      = std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery);
            vertexData[startVertexIndex + 3].pos[Y]      = std::sin(lat + kLatEvery);
            vertexData[startVertexIndex + 3].pos[Z]      = std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery);
            vertexData[startVertexIndex + 3].pos[W]      = 1.0f;
            vertexData[startVertexIndex + 3].texCoord[X] = float(lonIndex + 1) / divisionReal;
            vertexData[startVertexIndex + 3].texCoord[Y] = 1.0f - float(latIndex + 1) / divisionReal;
            vertexData[startVertexIndex + 3].normal      = {vertexData[startVertexIndex + 3].pos[X], vertexData[startIndexIndex + 3].pos[Y], vertexData[startIndexIndex + 3].pos[Z]};

            //
            startIndexIndex = (latIndex * division_ + lonIndex) * 6;

            indexData[startIndexIndex]     = startIndexIndex;
            indexData[startIndexIndex + 1] = startIndexIndex + 1;
            indexData[startIndexIndex + 2] = startIndexIndex + 2;

            indexData[startIndexIndex + 3] = startIndexIndex + 1;
            indexData[startIndexIndex + 4] = startIndexIndex + 3;
            indexData[startIndexIndex + 5] = startIndexIndex + 2;
        }
    }

    mesh_->TransferData();
}

/// =====================================================
/// Triangle
/// =====================================================
void Triangle::createMesh() {
    mesh_->vertexes_.clear();
    mesh_->indexes_.clear();

    // 頂点バッファにデータを格納
    mesh_->vertexes_.emplace_back(TextureVertexData(Vec4f(vertex_[0], 1.f), Vec2f(uv_[0], uv_[1]), normal_));
    mesh_->vertexes_.emplace_back(TextureVertexData(Vec4f(vertex_[1], 1.f), Vec2f(uv_[0], uv_[1]), normal_));
    mesh_->vertexes_.emplace_back(TextureVertexData(Vec4f(vertex_[2], 1.f), Vec2f(uv_[0], uv_[1]), normal_));

    // インデックスバッファにデータを格納
    mesh_->indexes_.emplace_back(0);
    mesh_->indexes_.emplace_back(1);
    mesh_->indexes_.emplace_back(2);

    mesh_->TransferData();
}
