#pragma once

/// stl
// memory
#include <memory>
// Container
#include <map>
#include <vector>
// basic class
#include <string>

/// engine
// dx12Object
#include "directX12/IConstantBuffer.h"
#include "directX12/Mesh.h"
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"
// lib
#include "Thread/Thread.h"
// math
#include "Matrix4x4.h"
#include "Quaternion.h"

struct TexturedMaterial {
    std::string texturePath = "";
    uint32_t textureNumber;
    IConstantBuffer<Material> material;
};

struct ModelNode {
    Matrix4x4 localMatrix;
    std::string name;
    std::vector<ModelNode> children;
};

struct ModelMeshData {
    LoadState currentState_ = LoadState::Unloaded;

    std::map<std::string, TextureMesh> meshGroup_;
    ModelNode rootNode;
};

struct Model {
    Model() = default;
    ~Model() {}
    ModelMeshData* meshData_;

    // Meshに対応した TransformBuffer
    std::map<TextureMesh*, Transform> transforms_;

    using ModelMaterialData = std::vector<TexturedMaterial>;
    ModelMaterialData materialData_;

    void setMaterialBuff(int32_t part, Material _data) {
        materialData_[part].material = _data;
    }
    /// <summary>
    /// 指定したメッシュのテクスチャをロードする
    /// </summary>
    /// <param name="part">メシュの番号</param>
    /// <param name="_texturePath">テクスチャパス</param>
    void loadTexture(int32_t part, const std::string& _texturePath);

    /// <summary>
    ///  <非推奨> 直接 指定したメッシュのテクスチャをセットする
    /// </summary>
    /// <param name="part"></param>
    /// <param name="_textureNumber"></param>
    void setTexture(int32_t part, uint32_t _textureNumber) {
        materialData_[part].texturePath   = "";
        materialData_[part].textureNumber = _textureNumber;
    }
};
