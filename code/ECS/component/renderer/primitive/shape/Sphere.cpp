#include "Sphere.h"

/// math
#include "math/MathEnv.h"

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
    // 緯度・経度それぞれ+1しているのは、UVの継ぎ目(シーム)を成立させるため。
    // 緯度方向は極(theta=0,pi)を含めてlatitudeDiv+1本のリングが必要で、
    // 経度方向はtheta=0とtheta=2piが同じ位置だが、UVのu座標(0と1)を別々に持たせたいので
    // 経度方向の最後にもう1列、始点と同じ位置の頂点を重複して生成する。
    vertexSize_ = static_cast<int32_t>((latitudeDiv + 1) * (longitudeDiv + 1));
    // 緯度・経度それぞれの分割区画(latitudeDiv x longitudeDiv 個)が1つの四角形になり、
    // それぞれを2枚の三角形(3頂点 x 2)で構成するため x6
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
    // theta: Y軸(上方向)からの傾き角(緯度)。0で北極(+Y)、piで南極(-Y)。
    // phi  : Y軸まわりの回転角(経度)。0~2piでZ軸(sinTheta*sinPhi項)を基準に一周する。
    // 球面座標系(theta, phi)から直交座標へ変換することで、緯度・経度のグリッド状に頂点を並べる。
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
            // 原点中心の球なので、位置ベクトルを正規化するだけで外向き法線になる
            Vec3f normal = pos.normalize();
            Vec2f uv     = {
                float(lon) / float(longitudeDiv),
                float(lat) / float(latitudeDiv)};

            _mesh->vertexes_.emplace_back(TextureColorVertexData(Vec4f(pos, 1.0f), uv, normal));
        }
    }

    // インデックス生成
    // 頂点は (longitudeDiv + 1) 個ずつの緯度リングが (latitudeDiv + 1) 本並んでいる配列。
    // current/next はそれぞれ現在の緯度リングと1つ南側の緯度リング上の同じ経度位置を指す。
    for (uint32_t lat = 0; lat < latitudeDiv; ++lat) {
        for (uint32_t lon = 0; lon < longitudeDiv; ++lon) {
            uint32_t current = lat * (longitudeDiv + 1) + lon;
            uint32_t next    = current + longitudeDiv + 1;

            // 2つの三角形で四角形を構成 (current, current+1, next, next+1 の順にCCWとなるよう巻く)
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
