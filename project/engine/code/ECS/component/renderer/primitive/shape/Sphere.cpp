#include "Sphere.h"

/// math
#include "math/mathEnv.h"

namespace OriGine {
namespace Primitive {

/// =====================================================
/// Sphere
/// ====================================================
void Sphere::CreateMesh(TextureColorMesh* _mesh) {
    // パラメータ
    const uint32_t latitudeDiv  = divisionLatitude_; // 緯度分割数
    const uint32_t longitudeDiv = divisionLongitude_; // 経度分割数
    const float radius          = radius_;

    // 頂点数・インデックス数を記録
    vertexSize_ = static_cast<int32_t>((latitudeDiv + 1) * (longitudeDiv + 1));
    indexSize_  = static_cast<int32_t>(latitudeDiv * longitudeDiv) * 6;

    // 頂点・インデックスバッファ初期化
    if ((int32_t)_mesh->GetIndexCapacity() < indexSize_) {
        // 必要なら Finalize
        if (_mesh->GetVertexBuffer().GetResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }
    // 頂点生成
    for (uint32_t lat = 0; lat <= latitudeDiv; ++lat) {
        float theta    = float(lat) * std::numbers::pi_v<float> / float(latitudeDiv); // 0 ~ pi
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (uint32_t lon = 0; lon <= longitudeDiv; ++lon) {
            float phi    = float(lon) * 2.0f * std::numbers::pi_v<float> / float(longitudeDiv); // 0 ~ 2pi
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            Vec3f pos = {
                radius * sinTheta * cosPhi,
                radius * cosTheta,
                radius * sinTheta * sinPhi};
            Vec3f normal = pos.normalize();
            Vec2f uv     = {
                float(lon) / float(longitudeDiv),
                float(lat) / float(latitudeDiv)};

            _mesh->vertexes_.emplace_back(TextureColorVertexData(Vec4f(pos, 1.0f), uv, normal));
        }
    }

    // インデックス生成
    for (uint32_t lat = 0; lat < latitudeDiv; ++lat) {
        for (uint32_t lon = 0; lon < longitudeDiv; ++lon) {
            uint32_t current = lat * (longitudeDiv + 1) + lon;
            uint32_t next    = current + longitudeDiv + 1;

            // 2つの三角形で四角形を構成
            _mesh->indexes_.emplace_back(current);
            _mesh->indexes_.emplace_back(current + 1);
            _mesh->indexes_.emplace_back(next);

            _mesh->indexes_.emplace_back(current + 1);
            _mesh->indexes_.emplace_back(next + 1);
            _mesh->indexes_.emplace_back(next);
        }
    }

    _mesh->TransferData();
}

}
}
