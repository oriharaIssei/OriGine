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
// assets
#include "material/Material.h"
// dx12Object
#include "directX12/IConstantBuffer.h"
#include "directX12/Mesh.h"
// component
#include "transform/Transform.h"
// lib
#include "Thread/Thread.h"
// math
#include "Matrix4x4.h"
#include "Quaternion.h"

struct TexturedMaterial {
    uint32_t textureNumber;
    IConstantBuffer<Material>* material;
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

struct Model { // Model から MeshModel に名称変更
    Model() = default;
    ~Model() {}
    ModelMeshData* meshData_;

    // Meshに対応した TransformBuffer
    std::map<TextureMesh*, Transform> transforms_;

    using ModelMaterialData = std::vector<TexturedMaterial>;
    ModelMaterialData materialData_;

    void setMaterialBuff(int32_t part, IConstantBuffer<Material>* buff) {
        materialData_[part].material = buff;
    }
    void setTexture(int32_t part, uint32_t textureNumber) {
        materialData_[part].textureNumber = textureNumber;
    }
};
