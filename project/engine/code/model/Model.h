#pragma once

/// stl
// memory
#include <memory>
// Container
#include <map>
#include <vector>
// basic class
#include <optional>
#include <string>

/// engine
// dx12Object
#include "directX12/IConstantBuffer.h"
#include "directX12/Mesh.h"
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

// math
#include "Matrix4x4.h"
#include "Quaternion.h"

struct TexturedMaterial {
    std::string texturePath = "";
    uint32_t textureNumber  = 0;

    IConstantBuffer<Material> material;
};

struct ModelNode {
    std::string name;

    Transform transform;
    Matrix4x4 localMatrix; // ローカル行列 (親の影響を受けない)
    std::vector<ModelNode> children;
};

struct Joint {
    std::string name = "Unknown";
    int32_t index    = -1; // Joint のインデックス

    Transform transform;
    Matrix4x4 localMatrix; // ローカル行列 (親の影響を受けない)
    Matrix4x4 skeletonSpaceMatrix; // スケルトン空間行列 (親の影響を受ける)

    std::vector<int32_t> children;
    std::optional<int32_t> parent;
};
struct Skeleton {
    int32_t rootJointIndex = -1; // ルートジョイントのインデックス
    std::map<std::string, int32_t> jointIndexBinder; // ジョイント名とインデックスのバインダー (名前からIndexを検索する)
    std::vector<Joint> joints; // 所属しているジョイント

    void Update() {
        for (Joint& joint : this->joints) {
            joint.localMatrix = MakeMatrix::Affine(joint.transform.scale, joint.transform.rotate, joint.transform.translate);

            if (joint.parent.has_value()) {
                joint.skeletonSpaceMatrix = joint.localMatrix * this->joints[*joint.parent].skeletonSpaceMatrix;
            } else {
                joint.skeletonSpaceMatrix = joint.localMatrix; // ルートジョイントはローカル行列がそのままスケルトン空間行列
            }
        }
    }
};

struct ModelMeshData {
    // LoadState currentState = LoadState::Unloaded;

    std::map<std::string, TextureMesh> meshGroup;

    ModelNode rootNode;
    std::optional<Skeleton> skeleton = std::nullopt; // スケルトンデータ
};

struct Model {
    Model() = default;
    ~Model() {}
    ModelMeshData* meshData_;

    // Meshに対応した Transform
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
