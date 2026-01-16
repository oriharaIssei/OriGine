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
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/IStructuredBuffer.h"
#include "directX12/mesh/Mesh.h"
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

// math
#include "Matrix4x4.h"
#include "Quaternion.h"

namespace OriGine {

/// <summary>
/// テクスチャとマテリアル情報をセットにした構造体.
/// </summary>
struct TexturedMaterial {
    /// <summary>テクスチャの相対パス</summary>
    std::string texturePath = "";
    /// <summary> texturePath からロードされたテクスチャの管理番号</summary>
    uint32_t textureNumber = 0;

    /// <summary>マテリアルの定数バッファ</summary>
    IConstantBuffer<Material> material;
};

/// <summary>
/// モデルの階層構造（ノード）を表す構造体.
/// </summary>
struct ModelNode {
    /// <summary>ノード名</summary>
    std::string name;

    /// <summary>ノードのトランスフォーム</summary>
    Transform transform;
    /// <summary>親の影響を含まないローカル行列</summary>
    Matrix4x4 localMatrix;
    /// <summary>子ノードのリスト</summary>
    std::vector<ModelNode> children;
};

/// <summary>
/// スケルトンを構成するジョイント名（ボーン）の情報.
/// </summary>
struct Joint {
    /// <summary>ジョイント名</summary>
    std::string name = "Unknown";
    /// <summary>Skeleton 内でのインデックス</summary>
    int32_t index = -1;

    /// <summary>デフォルトのトランスフォーム</summary>
    Transform transform;
    /// <summary>ローカル行列</summary>
    Matrix4x4 localMatrix;
    /// <summary>モデル（スケルトン）空間での行列</summary>
    Matrix4x4 skeletonSpaceMatrix;

    /// <summary>子ジョイントのインデックスリスト</summary>
    std::vector<int32_t> children;
    /// <summary>親ジョイントのインデックス</summary>
    std::optional<int32_t> parent;
};

/// <summary>
/// メッシュに関連付けられたジョイントの集合（スケルトン）.
/// </summary>
struct Skeleton {
    /// <summary>ルートとなるジョイントのインデックス</summary>
    int32_t rootJointIndex = -1;
    /// <summary>ジョイント名からインデックスを引くためのマップ</summary>
    std::map<std::string, int32_t> jointIndexBinder;
    /// <summary>所属する全ジョイントのリスト</summary>
    std::vector<Joint> joints;

    /// <summary>
    /// 全ジョイントの行列を階層に従って更新する.
    /// </summary>
    void Update();
};

/// <summary>
/// 特定のジョイントが頂点に与える影響度データ.
/// </summary>
struct VertexWeightData {
    /// <summary>影響度（0.0 ～ 1.0）</summary>
    float weight = 0.f;
    /// <summary>対象頂点のインデックス</summary>
    uint32_t vertexIndex = 0;
};

/// <summary>
/// ジョイントごとのウェイト情報の集合.
/// </summary>
struct JointWeightData {
    /// <summary>バインドポーズ座標系での逆行列</summary>
    Matrix4x4 inverseBindPoseMat;
    /// <summary>このジョイントが影響を与える全頂点のウェイトデータ</summary>
    std::vector<VertexWeightData> vertexWeights;
};

/// <summary>一頂点に影響を与える最大ジョイント数</summary>
constexpr uint32_t kNumMaxInfluence = 4;

/// <summary>
/// 頂点に対して影響を与えるジョイント情報（インデックスとウェイト）.
/// </summary>
struct VertexInfluence {
    /// <summary>各ジョイントのウェイト値</summary>
    Vector4f weights;
    /// <summary>各ジョイントのインデックス</summary>
    Vector4<int32_t> jointIndices;

public:
    /// <summary>GPU転送用の定数バッファ構造体</summary>
    struct ConstantBuffer {
        Vector4f weights;
        Vector4<int32_t> jointIndices;
        ConstantBuffer& operator=(const VertexInfluence& _influence) {
            weights      = _influence.weights;
            jointIndices = _influence.jointIndices;
            return *this;
        }
    };
};

/// <summary>
/// マトリクスパレットの一要素. 変形用行列と法線用行列を持つ.
/// </summary>
struct SkeletonMatrixWell {
    /// <summary>スケルトン空間行列</summary>
    Matrix4x4 skeletonSpaceMat;
    /// <summary>法線変換用の逆転置行列</summary>
    Matrix4x4 skeletonSpaceInverseTransposeMat;

    /// <summary>GPU転送用の定数バッファ構造体</summary>
    struct ConstantBuffer {
        Matrix4x4 skeletonSpaceMat;
        Matrix4x4 normalSpaceMat;

    public:
        ConstantBuffer& operator=(const SkeletonMatrixWell& _well) {
            skeletonSpaceMat = _well.skeletonSpaceMat;
            normalSpaceMat   = _well.skeletonSpaceInverseTransposeMat;
            return *this;
        }
    };
};

/// <summary>
/// スキニング処理に関する全体情報.
/// </summary>
struct SkinningInfo {
    /// <summary>スキニング対象の頂点数</summary>
    std::uint32_t vertexSize = 0;

    /// <summary>GPU転送用の定数バッファ構造体</summary>
    struct ConstantBuffer {
        std::uint32_t vertexSize = 0;
        ConstantBuffer& operator=(const SkinningInfo& _info) {
            vertexSize = _info.vertexSize;
            return *this;
        }
    };
};

/// <summary>
/// スキニングに必要なバッファ群を保持・管理する構造体.
/// </summary>
struct SkinCluster {
    /// <summary>各ジョイントのバインドポーズ逆行列リスト</summary>
    std::vector<Matrix4x4> inverseBindPoseMatrices;

    /// <summary>全頂点のウェイト・インデックス情報を格納する構造化バッファ</summary>
    IStructuredBuffer<VertexInfluence> vertexInfluencesBuffer_;
    /// <summary>現在のボーン行列群（パレット）を格納する構造化バッファ</summary>
    IStructuredBuffer<SkeletonMatrixWell> skeletonMatrixPaletteBuffer_;
    /// <summary>スキニングのメタ情報を格納する定数バッファ</summary>
    IConstantBuffer<SkinningInfo> skinningInfoBuffer_;

    /// <summary>
    /// 現在のスケルトンの状態に基づいてマトリクスパレットを計算し、バッファを更新する.
    /// </summary>
    /// <param name="_skeleton">更新に使用するスケルトンデータ</param>
    void UpdateMatrixPalette(const Skeleton& _skeleton);
};

/// <summary>
/// モデルを構成するメッシュやスキンデータの静的なリソース集合.
/// モデルマネージャによって管理され、複数の Model インスタンスから参照される.
/// </summary>
struct ModelMeshData {
    /// <summary>メッシュ名とメッシュオブジェクト（頂点・インデックス）のマップ</summary>
    std::map<std::string, TextureColorMesh> meshGroup;
    /// <summary>メッシュ名とジョイントウェイトデータのマップ（読み込み用）</summary>
    std::map<std::string, JointWeightData> jointWeightData;
    /// <summary>メッシュ名とスキンクラスター（GPUバッファ）のマップ</summary>
    std::map<std::string, SkinCluster> skinClusterDataMap;

    /// <summary>ノード構成のルート</summary>
    ModelNode rootNode;
    /// <summary>ボーン構造データ（任意）</summary>
    std::optional<Skeleton> skeleton = std::nullopt;
};

/// <summary>
/// シーン上に存在する個別のモデルを表すクラス.
/// メッシュデータ自体は ModelMeshData への参照として保持する.
/// </summary>
struct Model {
    Model() = default;
    ~Model() {}

    /// <summary>共有されるメッシュデータへのポインタ</summary>
    ModelMeshData* meshData_;

    /// <summary>各メッシュパーツに対応するトランスフォーム情報</summary>
    std::map<TextureColorMesh*, Transform> transforms_;

    /// <summary>インスタンス固有のマテリアル・テクスチャデータのリスト</summary>
    using ModelMaterialData = std::vector<TexturedMaterial>;
    ModelMaterialData materialData_;

    /// <summary>
    /// 指定したパーツのマテリアル情報を設定する.
    /// </summary>
    /// <param name="_part">メッシュのパーツインデックス</param>
    /// <param name="_data">設定するマテリアルデータ</param>
    void SetMaterialBuff(int32_t _part, Material _data) {
        materialData_[_part].material.openData_ = _data;
    }

    /// <summary>
    /// 指定したパーツに使用するテクスチャをファイルからロードする.
    /// </summary>
    /// <param name="_part">メッシュのパーツインデックス</param>
    /// <param name="_texturePath">テクスチャのパス</param>
    void LoadTexture(int32_t _part, const std::string& _texturePath);

    /// <summary>
    /// 指定したパーツに使用するテクスチャを管理番号で直接設定する.
    /// </summary>
    /// <param name="_part">メッシュのパーツインデックス</param>
    /// <param name="_textureNumber">テクスチャ管理番号</param>
    void SetTexture(int32_t _part, uint32_t _textureNumber) {
        materialData_[_part].texturePath   = "";
        materialData_[_part].textureNumber = _textureNumber;
    }
};

} // namespace OriGine
