#pragma once

/// stl
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// engine
#include "directX12/BlendMode.h"
#include "directX12/buffer/IStructuredBuffer.h"
#include "directX12/buffer/SimpleConstantBuffer.h"
#include "directX12/instancing/InstanceHandle.h"

/// util
#include "container/DenseSlotMap.h"

/// ECS
#include "component/material/Material.h"
#include "component/transform/Transform.h"
#include "entity/EntityHandle.h"

/// model
#include "model/Model.h"

namespace OriGine {

/// 前方宣言
class Scene;

/// <summary>
/// インスタンシング描画における1インスタンス分のデータ.
/// </summary>
struct InstanceEntry {
    /// <summary>所有エンティティ</summary>
    EntityHandle owner;
    /// <summary>サブメッシュ毎のマテリアル</summary>
    std::vector<Material> materials;
    /// <summary>サブメッシュ毎のテクスチャ番号</summary>
    std::vector<size_t> textureIndices;
    /// <summary>サブメッシュ毎のローカル Transform（インスタンス固有。未設定時はグループのデフォルトを使用）</summary>
    std::vector<Transform> localTransforms;
    /// <summary>サブメッシュ毎のカスタムテクスチャ GPU ハンドル（.ptr==0 で無効→textureIndices を使用）</summary>
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> customTextureHandles;
    /// <summary>描画時のブレンドモード</summary>
    BlendMode blendMode = BlendMode::Alpha;
    /// <summary>描画時のカリング設定</summary>
    bool isCulling = true;
};

/// <summary>
/// テクスチャ別の描画バッチ情報.
/// 同一テクスチャのインスタンスをまとめて1回の DrawIndexedInstanced で描画する.
/// </summary>
struct TextureBatch {
    D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = {};
    uint32_t startInstance = 0;
    uint32_t instanceCount = 0;
    BlendMode blendMode    = BlendMode::Alpha;
    bool isCulling         = true;
};

/// <summary>
/// 同一モデル（ModelMeshData）を共有するインスタンス群と、
/// インスタンシング描画に必要な GPU バッファをまとめた構造体.
/// </summary>
struct InstancedModelGroup {
    /// <summary>共有されるモデルデータへのポインタ</summary>
    ModelMeshData* modelData = nullptr;

    /// <summary>インスタンスの管理コンテナ（Handle ベース）</summary>
    DenseSlotMap<InstanceEntry> instances;

    /// <summary>サブメッシュ一覧（ModelNode 走査順）</summary>
    std::vector<TextureColorMesh*> submeshOrder;
    /// <summary>サブメッシュ毎のローカル Transform（ModelNode 由来）</summary>
    std::vector<Transform> submeshLocalTransforms;

    /// <summary>サブメッシュ毎の Transform StructuredBuffer（GPU用）</summary>
    std::vector<IStructuredBuffer<Transform>> transformBuffers;
    /// <summary>サブメッシュ毎の Material StructuredBuffer（GPU用）</summary>
    std::vector<IStructuredBuffer<Material>> materialBuffers;

    /// <summary>サブメッシュ毎のテクスチャバッチ情報（UpdateBuffers で構築、Render で使用）</summary>
    std::vector<std::vector<TextureBatch>> textureBatches;
};

/// <summary>
/// インスタンシング描画を管理するクラス.
/// 同一モデルの複数インスタンスを DenseSlotMap で管理し、
/// StructuredBuffer 経由で GPU にまとめて転送することで
/// DrawIndexedInstanced を1回のドローコールにまとめる.
/// </summary>
class InstancedMeshManager {
public:
    /// <summary>StructuredBuffer の初期インスタンス容量</summary>
    static constexpr uint32_t kDefaultInstanceCapacity = 64;
    /// <summary>リサイズ時の倍率</summary>
    static constexpr uint32_t kGrowFactor = 2;

    InstancedMeshManager()  = default;
    ~InstancedMeshManager() = default;

    /// <summary>初期化</summary>
    void Initialize();

    /// <summary>終了処理。全バッファを解放する.</summary>
    void Finalize();

    /// <summary>
    /// インスタンスを追加し、ハンドルを返す.
    /// </summary>
    /// <param name="_modelData">共有モデルデータ（ModelManager 由来）</param>
    /// <param name="_owner">所有エンティティ</param>
    /// <returns>発行されたインスタンスハンドル</returns>
    InstanceHandle AddInstance(ModelMeshData* _modelData, EntityHandle _owner);

    /// <summary>
    /// ハンドルを指定してインスタンスを削除する.
    /// </summary>
    /// <param name="_handle">削除対象のハンドル</param>
    void RemoveInstance(const InstanceHandle& _handle);

    /// <summary>
    /// ハンドルからインスタンスデータへの参照を取得する.
    /// マテリアルやテクスチャの変更に使用する.
    /// </summary>
    /// <param name="_handle">対象のハンドル</param>
    /// <returns>インスタンスデータへの参照</returns>
    InstanceEntry& GetInstance(const InstanceHandle& _handle);

    /// <summary>ハンドルが有効かどうかを返す</summary>
    bool IsValid(const InstanceHandle& _handle) const;

    /// <summary>管理しているインスタンスが存在しないかどうかを返す</summary>
    bool IsEmpty() const;

    /// <summary>
    /// 毎フレーム呼び出し。各インスタンスの Transform / Material を
    /// StructuredBuffer に転送する.
    /// </summary>
    /// <param name="_scene">Transform 取得用のシーン</param>
    void UpdateBuffers(Scene* _scene);

    /// <summary>
    /// 指定された BlendMode / Culling に一致するインスタンスのみ描画する.
    /// PSO 等は呼び出し側でセット済みであること.
    /// </summary>
    /// <param name="_cmdList">コマンドリスト</param>
    /// <param name="_transformBufferIndex">Transform StructuredBuffer のルートパラメータ番号</param>
    /// <param name="_materialBufferIndex">Material StructuredBuffer のルートパラメータ番号</param>
    /// <param name="_textureBufferIndex">テクスチャのルートパラメータ番号</param>
    /// <param name="_offsetBufferIndex">インスタンスオフセットのルートパラメータ番号</param>
    /// <param name="_blendMode">描画対象のブレンドモード</param>
    /// <param name="_isCulling">描画対象のカリング設定</param>
    void Render(
        const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _cmdList,
        int32_t _transformBufferIndex,
        int32_t _materialBufferIndex,
        int32_t _textureBufferIndex,
        int32_t _offsetBufferIndex,
        BlendMode _blendMode,
        bool _isCulling);

    /// <summary>
    /// 指定された BlendMode / Culling に一致するインスタンスが存在するか.
    /// </summary>
    bool HasInstancesFor(BlendMode _blendMode, bool _isCulling) const;

    /// <summary>
    /// プリミティブ型用のテンプレートメッシュを取得または作成する.
    /// 同一 typeName で2回目以降はキャッシュを返す.
    /// </summary>
    /// <param name="_typeName">プリミティブ型名（例: "Plane", "Box"）</param>
    /// <param name="_meshCreator">テンプレートメッシュを生成するコールバック</param>
    /// <returns>テンプレート ModelMeshData へのポインタ</returns>
    ModelMeshData* GetOrCreatePrimitiveTemplate(
        const std::string& _typeName,
        std::function<void(TextureColorMesh*)> _meshCreator);

private:
    /// <summary>モデルデータ別のインスタンスグループ</summary>
    std::unordered_map<ModelMeshData*, InstancedModelGroup> modelGroups_;

    /// <summary>プリミティブ型用テンプレートメッシュ（型名 → ModelMeshData）</summary>
    std::unordered_map<std::string, std::unique_ptr<ModelMeshData>> primitiveTemplates_;

    /// <summary>[culling(0/1)][blendMode] 該当バッチが存在するかのキャッシュ（UpdateBuffers で構築）</summary>
    std::array<std::array<bool, kBlendNum>, 2> hasInstanceFlags_{};

    /// <summary>
    /// ModelNode を再帰走査して submesh の順序とローカル Transform を構築する.
    /// </summary>
    /// <param name="_group">構築対象のグループ</param>
    void BuildSubmeshOrder(InstancedModelGroup& _group);

    /// <summary>
    /// StructuredBuffer の容量が不足している場合にリサイズする.
    /// </summary>
    /// <param name="_group">対象グループ</param>
    /// <param name="_requiredCount">必要なインスタンス数</param>
    void EnsureCapacity(InstancedModelGroup& _group, uint32_t _requiredCount);

    /// <summary>
    /// UpdateBuffers 完了後にバッチ情報から hasInstanceFlags_ を再構築する.
    /// </summary>
    void RebuildInstanceFlags();
};

} // namespace OriGine
