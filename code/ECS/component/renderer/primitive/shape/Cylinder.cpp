#include "Cylinder.h"

namespace OriGine {
namespace Primitive {
void Cylinder::CreateMesh(TextureColorMesh* _mesh) {

    // 頂点数とインデックス数の設定
    // 円周方向をradialDivisions+1にしているのは、Sphere同様に始点(角度0)と終点(角度2pi)を
    // 別頂点として持たせ、UVのu座標を0→1で継ぎ目なく貼れるようにするため(継ぎ目=シーム)。
    // 高さ方向もheightDivisions+1本のリングが必要(区間の両端を含むため)。
    vertexSize_ = (radialDivisions + 1) * (heightDivisions + 1);
    // 側面: 高さ・円周それぞれの分割区画(heightDivisions x radialDivisions)を四角形とし、
    //       各四角形を2枚の三角形(3頂点)で構成するため x6。
    // 末尾の "+ radialDivisions * 6" は上面・底面の蓋を構成する分の予約
    // (実際にキャップを生成する処理は本関数には無いため、将来の拡張分を見越した余剰確保)。
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

    // 角度ステップ (kTau = 2pi。円周をradialDivisions等分する1区画あたりの角度)
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

        float v      = float(h) / float(heightDivisions); // 高さ方向の正規化パラメータ(0~1)
        // 半径の変化に緩急を付けられるよう、線形補間の前にイージング関数でvを変形する
        // (radiusEaseTypeがLinearなら通常のLerpと同じ結果になる)
        float easedV = EasingFunctions[radiusEaseTypeInt](v);

        // 底面から上面へ向けて半径をイージング補間(topRadius/bottomRadiusはXZ方向で別々に指定できる楕円対応)
        Vec2f radius = Lerp(bottomRadius, topRadius, easedV);
        float y      = std::lerp(0.0f, height, v); // 高さ自体は常に線形補間

        for (uint32_t r = 0; r <= radialDivisions; ++r) {

            // Y軸まわりにangle分回転させたXZ平面上の単位円座標(sin, cos)
            float angle = angleStep * float(r);
            float s     = sinf(angle);
            float c     = cosf(angle);

            Vec3f pos(s * radius[X], y, c * radius[Y]);

            float u = float(r) / float(radialDivisions);

            TextureColorMesh::VertexType vertex{};
            vertex.pos      = Vec4f(pos, 1.f);
            // 側面法線は円周上の半径方向(Y成分なし)。上下で半径が異なるテーパー形状でも
            // 近似的にXZ方向を向く法線として扱う(円錐台の正確な傾斜法線ではない簡易版)
            vertex.normal   = Vec3f(s, 0.0f, c); // 側面法線
            vertex.texCoord = Vec2f(u, v);

            vertices.emplace_back(vertex);
        }
    }

    // ==============================
    // インデックス生成
    // ==============================
    uint32_t ringVertexCount = radialDivisions + 1; // 1つの高さリングを構成する頂点数(円周方向+1)

    for (uint32_t h = 0; h < heightDivisions; ++h) {
        for (uint32_t r = 0; r < radialDivisions; ++r) {

            // i0-i1が現在の高さリング、i2-i3が1段上の高さリングの、同じ円周位置の頂点
            uint32_t i0 = h * ringVertexCount + r;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + ringVertexCount;
            uint32_t i3 = i2 + 1;

            // i0,i1,i2,i3を頂点とする四角形を2枚の三角形に分割(いずれも外側から見てCCWになる巻き順)
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
