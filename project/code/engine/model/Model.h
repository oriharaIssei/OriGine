#pragma once

#include "Matrix4x4.h"
#include "Quaternion.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/Object3dMesh.h"
#include "material/Material.h"
#include "transform/Transform.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

struct Material3D {
    uint32_t textureNumber;
    IConstantBuffer<Material>* material;
};

struct ModelNode {
    Matrix4x4 localMatrix;
    std::string name;
    std::vector<ModelNode> children;
};

struct Mesh3D {
    std::unique_ptr<IObject3dMesh> meshBuff;
    size_t dataSize  = 0;
    size_t vertSize  = 0;
    size_t indexSize = 0;

    IConstantBuffer<Transform> transform_;
    // 対応するノードの名前
    std::string nodeName;
};

struct ModelMeshData {
    std::unordered_map<std::string, uint32_t> meshIndexes;
    std::vector<Mesh3D> mesh_;
    ModelNode rootNode;
};

struct Model {
    enum class LoadState {
        Loading,
        Loaded,
    };
    LoadState currentState_ = LoadState::Loading;

    ModelMeshData* meshData_;

    using ModelMaterialData = std::vector<Material3D>;
    ModelMaterialData materialData_;

    void setMaterialBuff(int32_t part, IConstantBuffer<Material>* buff) {
        materialData_[part].material = buff;
    }
    void setTexture(int32_t part, uint32_t textureNumber) {
        materialData_[part].textureNumber = textureNumber;
    }
};
