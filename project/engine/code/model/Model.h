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
#include "directX12/IStructuredBuffer.h"
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

    void Update();
};

struct VertexWeightData {
    float weight         = 0.f; // ウェイト
    uint32_t vertexIndex = 0; // 対応する頂点のインデックス
};
struct JointWeightData {
    Matrix4x4 inverseBindPoseMat; // バインドポーズ座標系での逆行列
    std::vector<VertexWeightData> vertexWeights; // 対応する頂点のウェイトデータ
};

constexpr uint32_t kNumMaxInfluence = 4; // 影響を与えるJointの最大数
/// <summary>
/// 頂点に対して影響を与えるデータ群
/// </summary>
struct VertexInfluence {
    Vector4f weights; // 各ジョイントのウェイト
    Vector4<int32_t> jointIndices; // 各ジョイントのインデックス

public:
    struct ConstantBuffer {
        Vector4f weights; // 各ジョイントのウェイト
        Vector4<int32_t> jointIndices; // 各ジョイントのインデックス
        ConstantBuffer& operator=(const VertexInfluence& influence) {
            weights      = influence.weights;
            jointIndices = influence.jointIndices;
            return *this;
        }
    };
};
/// <summary>
/// SkeletonMatrixPalette の 1要素
/// </summary>
struct SkeletonMatrixWell {
    Matrix4x4 skeletonSpaceMat; // 座標用
    Matrix4x4 skeletonSpaceInverseTransposeMat; // 法線用
    struct ConstantBuffer {
        Matrix4x4 skeletonSpaceMat; // 座標用
        Matrix4x4 normalSpaceMat; // 法線用

    public:
        ConstantBuffer& operator=(const SkeletonMatrixWell& well) {
            skeletonSpaceMat = well.skeletonSpaceMat;
            normalSpaceMat   = well.skeletonSpaceInverseTransposeMat;
            return *this;
        }
    };
};
struct SkinningInfo {
    std::uint32_t vertexSize = 0; // スキニングされる頂点の数

    struct ConstantBuffer {
        std::uint32_t vertexSize = 0; // スキニングされる頂点の数
        ConstantBuffer& operator=(const SkinningInfo& info) {
            vertexSize = info.vertexSize;
            return *this;
        }
    };
};

struct SkinCluster {
    std::vector<Matrix4x4> inverseBindPoseMatrices; // バインドポーズ座標系での逆行列 郡

    IStructuredBuffer<VertexInfluence> vertexInfluencesBuffer_; // 頂点に対する影響を与えるデータ群
    IStructuredBuffer<SkeletonMatrixWell> skeletonMatrixPaletteBuffer_; // スケルトンマトリクスパレット
    IConstantBuffer<SkinningInfo> skinningInfoBuffer_; // スキニング情報

    void UpdateMatrixPalette(const Skeleton& _skeleton);
};

struct ModelMeshData {
    // LoadState currentState = LoadState::Unloaded;

    // 各メッシュの頂点データ
    std::map<std::string, TextureMesh> meshGroup;
    // スキンメッシュのスキンクラスターデータ
    std::map<std::string, JointWeightData> jointWeightData;
    std::map<std::string, SkinCluster> skinClusterDataMap;

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
