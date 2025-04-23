#include "Primitive.h"

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include <imgui/imgui.h>
#endif // _DEBUG

/// =====================================================
/// Plane
/// =====================================================
void Plane::createMesh(TextureMesh* _mesh) {
    // XZ 平面を作成
    _mesh->setVertexData({{Vec4f(-size_[X], size_[Y], 0.0f, 1.0f), Vec2f(0.0f, 0.0f), normal_},
        {Vec4f(size_[X], size_[Y], 0.0f, 1.0f), Vec2f(uv_[X], 0.0f), normal_},
        {Vec4f(-size_[X], -size_[Y], 0.0f, 1.0f), Vec2f(0.0f, uv_[Y]), normal_},
        {Vec4f(size_[X], -size_[Y], 0.0f, 1.0f), Vec2f(uv_[X], uv_[Y]), normal_}});
    _mesh->setIndexData(
        {0, 1, 2,
            1, 3, 2});
    _mesh->TransferData();
}

/// =====================================================
/// Circle
/// =====================================================
void Circle::createMesh(TextureMesh* _mesh) {
    _mesh->vertexes_.clear();
    _mesh->indexes_.clear();
    // 円の頂点を計算
    for (uint32_t i = 0; i < vertexSize_; ++i) {
        float angle    = static_cast<float>(i) * 2.0f * std::numbers::pi_v<float> / static_cast<float>(vertexSize_);
        Vector3f point = {radius_ * std::cos(angle), 0.f, radius_ * std::sin(angle)};
        _mesh->vertexes_.emplace_back(TextureVertexData(Vec4f(point, 1.f), Vec2f(), point.normalize()));
    }
    // 円のインデックスを計算
    for (uint32_t i = 0; i < indexSize_; ++i) {
        _mesh->indexes_.emplace_back(i);
    }
    _mesh->TransferData();
}

/// =====================================================
/// Ring
/// =====================================================
void Ring::createMesh(TextureMesh* _mesh) {
    _mesh->vertexes_.clear();
    _mesh->indexes_.clear();

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
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sin * outerRadius_, cos * outerRadius_, 0.f, 1.f), Vec2f(u, 0.f), Vector3f(0.f, 0.f, -1.f)));
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sinNext * outerRadius_, cosNext * outerRadius_, 0.f, 1.f), Vec2f(uNext, 0.f), Vector3f(0.f, 0.f, -1.f)));
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sin * innerRadius_, cos * innerRadius_, 0.f, 1.f), Vec2f(u, 1.f), Vector3f(0.f, 0.f, -1.f)));
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sinNext * innerRadius_, cosNext * innerRadius_, 0.f, 1.f), Vec2f(uNext, 1.f), Vector3f(0.f, 0.f, -1.f)));

        // Index
        uint32_t startIndex = i * 4;
        _mesh->indexes_.emplace_back(startIndex);
        _mesh->indexes_.emplace_back(startIndex + 1);
        _mesh->indexes_.emplace_back(startIndex + 2);
        _mesh->indexes_.emplace_back(startIndex + 1);
        _mesh->indexes_.emplace_back(startIndex + 3);
        _mesh->indexes_.emplace_back(startIndex + 2);
    }

    _mesh->TransferData();
}

/// =====================================================
/// Sphere
/// =====================================================
// void Sphere::createMesh(TextureMesh* _mesh) {
//     _mesh->vertexes_.clear();
//     _mesh->indexes_.clear();
//
//     auto& vertexData = _mesh->vertexes_;
//     auto& indexData  = _mesh->indexes_;
//
//     float divisionReal    = static_cast<float>(division_);
//     const float kLatEvery = std::numbers::pi_v<float> / divisionReal; // 緯度
//     const float kLonEvery = 2.0f * std::numbers::pi_v<float> / divisionReal; // 経度
//
//     uint32_t startVertexIndex = 0;
//     uint32_t startIndexIndex  = 0;
//
//     for (uint32_t latIndex = 0; latIndex < division_; ++latIndex) {
//         float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; // theta
//         // 経度方向に分割
//         for (uint32_t lonIndex = 0; lonIndex < division_; ++lonIndex) {
//             startVertexIndex = (latIndex * division_ + lonIndex) * 4;
//             float lon        = lonIndex * kLonEvery; // fai
//             // 頂点データを入力
//             // a 左下
//             vertexData[startVertexIndex].pos[0]      = std::cos(lat) * std::cos(lon);
//             vertexData[startVertexIndex].pos[1]      = std::sin(lat);
//             vertexData[startVertexIndex].pos[2]      = std::cos(lat) * std::sin(lon);
//             vertexData[startVertexIndex].pos[3]      = 1.0f;
//             vertexData[startVertexIndex].texCoord[0] = float(lonIndex) / divisionReal;
//             vertexData[startVertexIndex].texCoord[1] = 1.0f - float(latIndex) / divisionReal;
//             vertexData[startVertexIndex].normal      = {vertexData[startVertexIndex].pos[X], vertexData[startVertexIndex].pos[Y], vertexData[startVertexIndex].pos[Z]};
//
//             // b 左上
//             vertexData[startVertexIndex + 1].pos[0]      = std::cos(lat + kLatEvery) * std::cos(lon);
//             vertexData[startVertexIndex + 1].pos[1]      = std::sin(lat + kLatEvery);
//             vertexData[startVertexIndex + 1].pos[2]      = std::cos(lat + kLatEvery) * std::sin(lon);
//             vertexData[startVertexIndex + 1].pos[3]      = 1.0f;
//             vertexData[startVertexIndex + 1].texCoord[0] = float(lonIndex) / divisionReal;
//             vertexData[startVertexIndex + 1].texCoord[1] = 1.0f - float(latIndex + 1) / divisionReal;
//             vertexData[startVertexIndex + 1].normal      = {vertexData[startVertexIndex + 1].pos[X], vertexData[startVertexIndex + 1].pos[Y], vertexData[startVertexIndex + 1].pos[Z]};
//
//             // c 右下
//             vertexData[startVertexIndex + 2].pos[X]      = std::cos(lat) * std::cos(lon + kLonEvery);
//             vertexData[startVertexIndex + 2].pos[Y]      = std::sin(lat);
//             vertexData[startVertexIndex + 2].pos[Z]      = std::cos(lat) * std::sin(lon + kLonEvery);
//             vertexData[startVertexIndex + 2].pos[W]      = 1.0f;
//             vertexData[startVertexIndex + 2].texCoord[X] = float(lonIndex + 1) / divisionReal;
//             vertexData[startVertexIndex + 2].texCoord[Y] = 1.0f - float(latIndex) / divisionReal;
//             vertexData[startVertexIndex + 2].normal      = {vertexData[startVertexIndex + 2].pos[X], vertexData[startVertexIndex + 2].pos[Y], vertexData[startVertexIndex + 2].pos[Z]};
//
//             // d 右上
//             vertexData[startVertexIndex + 3].pos[X]      = std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery);
//             vertexData[startVertexIndex + 3].pos[Y]      = std::sin(lat + kLatEvery);
//             vertexData[startVertexIndex + 3].pos[Z]      = std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery);
//             vertexData[startVertexIndex + 3].pos[W]      = 1.0f;
//             vertexData[startVertexIndex + 3].texCoord[X] = float(lonIndex + 1) / divisionReal;
//             vertexData[startVertexIndex + 3].texCoord[Y] = 1.0f - float(latIndex + 1) / divisionReal;
//             vertexData[startVertexIndex + 3].normal      = {vertexData[startVertexIndex + 3].pos[X], vertexData[startIndexIndex + 3].pos[Y], vertexData[startIndexIndex + 3].pos[Z]};
//
//             //
//             startIndexIndex = (latIndex * division_ + lonIndex) * 6;
//
//             indexData[startIndexIndex]     = startIndexIndex;
//             indexData[startIndexIndex + 1] = startIndexIndex + 1;
//             indexData[startIndexIndex + 2] = startIndexIndex + 2;
//
//             indexData[startIndexIndex + 3] = startIndexIndex + 1;
//             indexData[startIndexIndex + 4] = startIndexIndex + 3;
//             indexData[startIndexIndex + 5] = startIndexIndex + 2;
//         }
//     }
//
//     _mesh->TransferData();
// }

/// =====================================================
/// Triangle
/// =====================================================
// void Triangle::createMesh(TextureMesh* _mesh) {
//     _mesh->vertexes_.clear();
//     _mesh->indexes_.clear();
//
//     // 頂点バッファにデータを格納
//     _mesh->vertexes_.emplace_back(TextureVertexData(Vec4f(vertex_[0], 1.f), Vec2f(uv_[0], uv_[1]), normal_));
//     _mesh->vertexes_.emplace_back(TextureVertexData(Vec4f(vertex_[1], 1.f), Vec2f(uv_[0], uv_[1]), normal_));
//     _mesh->vertexes_.emplace_back(TextureVertexData(Vec4f(vertex_[2], 1.f), Vec2f(uv_[0], uv_[1]), normal_));
//
//     // インデックスバッファにデータを格納
//     _mesh->indexes_.emplace_back(0);
//     _mesh->indexes_.emplace_back(1);
//     _mesh->indexes_.emplace_back(2);
//
//     _mesh->TransferData();
// }

bool PlaneRenderer::Edit() {
#ifdef _DEBUG
    bool isEdit = false;

    ImGui::Text("Texture Directory : %s", textureDirectory_.c_str());
    ImGui::Text("Texture FileName : %s", textureFileName_.c_str());
    if (ImGui::Button("LoadTexture")) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            textureDirectory_ = kApplicationResourceDirectory + "/" + directory;
            textureFileName_  = fileName;

            // テクスチャの読み込み
            textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_);

            isEdit = true;
        }
    }

    if (ImGui::TreeNode("Transform")) {
        if (transformBuff_.openData_.Edit()) {
            transformBuff_.ConvertToBuffer();
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Material")) {
        materialBuff_.openData_.DebugGui();
        ImGui::TreePop();
    }

    return isEdit;
#else
    return false;
#endif // _DEBUG
}

void PlaneRenderer::Save(BinaryWriter& _writer) {
    MeshRenderer::Save(_writer);

    _writer.Write<std::string>("textureDirectory", textureDirectory_);
    _writer.Write<std::string>("textureFileName", textureFileName_);

    transformBuff_.openData_.Save(_writer);

    _writer.Write<4, float>("color", materialBuff_.openData_.color_);
    _writer.Write<3, float>("uvScale", materialBuff_.openData_.uvScale_);
    _writer.Write<3, float>("uvRotate", materialBuff_.openData_.uvRotate_);
    _writer.Write<3, float>("uvTranslate", materialBuff_.openData_.uvTranslate_);
    _writer.Write<int32_t>("enableLighting", materialBuff_.openData_.enableLighting_);
    _writer.Write<float>("shininess", materialBuff_.openData_.shininess_);
    _writer.Write<3, float>("specularColor", materialBuff_.openData_.specularColor_);
}

void PlaneRenderer::Load(BinaryReader& _reader) {
    MeshRenderer::Load(_reader);

    _reader.Read<std::string>("textureDirectory", textureDirectory_);
    _reader.Read<std::string>("textureFileName", textureFileName_);
    if (!textureFileName_.empty()) { // テクスチャの読み込み
        textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_);
    }

    transformBuff_.openData_.Load(_reader);

    _reader.Read<4, float>("color", materialBuff_.openData_.color_);
    _reader.Read<3, float>("uvScale", materialBuff_.openData_.uvScale_);
    _reader.Read<3, float>("uvRotate", materialBuff_.openData_.uvRotate_);
    _reader.Read<3, float>("uvTranslate", materialBuff_.openData_.uvTranslate_);
    _reader.Read<int32_t>("enableLighting", materialBuff_.openData_.enableLighting_);
    _reader.Read<float>("shininess", materialBuff_.openData_.shininess_);
    _reader.Read<3, float>("specularColor", materialBuff_.openData_.specularColor_);
}
