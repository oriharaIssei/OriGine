#include "Primitive.h"

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

void Sphere::createMesh() {
    mesh_->vertexes_.clear();
    mesh_->indexes_.clear();

    float divisionReal    = static_cast<float>(division_);
    const float kLatEvery = std::numbers::pi_v<float> / divisionReal; //* 緯度
    const float kLonEvery = 2.0f * std::numbers::pi_v<float> / divisionReal; //* 経度

    auto calculatePoint = [&](float lat, float lon) -> Vector3f {
        return {
            radius_[X] * std::cos(lat) * std::cos(lon),
            radius_[Y] * std::sin(lat),
            radius_[Z] * std::cos(lat) * std::sin(lon)};
    };

    // 緯線（緯度方向の円）を描画
    for (uint32_t latIndex = 1; latIndex < division_; ++latIndex) {
        float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
        for (uint32_t lonIndex = 0; lonIndex < division_; ++lonIndex) {
            float lonA = lonIndex * kLonEvery;
            float lonB = (lonIndex + 1) % division_ * kLonEvery;

            Vector3f pointA = calculatePoint(lat, lonA);
            Vector3f pointB = calculatePoint(lat, lonB);

            // 頂点バッファにデータを 格納
            mesh_->vertexes_.emplace_back(TextureVertexData(Vec4f(pointA, 1.f), Vec2f(), pointA.normalize()));
            mesh_->vertexes_.emplace_back(TextureVertexData(Vec4f(pointB, 1.f), Vec2f(), pointB.normalize()));
            // インデックスバッファにデータを格納
            mesh_->indexes_.emplace_back(uint32_t(mesh_->indexes_.size()));
            mesh_->indexes_.emplace_back(uint32_t(mesh_->indexes_.size()));
        }
    }

    // 経線（経度方向の円）を描画
    for (uint32_t lonIndex = 0; lonIndex < division_; ++lonIndex) {
        float lon = lonIndex * kLonEvery;
        for (uint32_t latIndex = 0; latIndex < division_; ++latIndex) {
            float latA = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
            float latB = -std::numbers::pi_v<float> / 2.0f + kLatEvery * (latIndex + 1);

            Vector3f pointA = calculatePoint(latA, lon);
            Vector3f pointB = calculatePoint(latB, lon);

            // 頂点バッファにデータを格納
            mesh_->vertexes_.emplace_back(TextureVertexData(Vec4f(pointA, 1.f), Vec2f(), pointA.normalize()));
            mesh_->vertexes_.emplace_back(TextureVertexData(Vec4f(pointB, 1.f), Vec2f(), pointB.normalize()));
            // インデックスバッファにデータを格納
            mesh_->indexes_.emplace_back(uint32_t(mesh_->indexes_.size()));
            mesh_->indexes_.emplace_back(uint32_t(mesh_->indexes_.size()));
        }
    }

    mesh_->TransferData();
}
