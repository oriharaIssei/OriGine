#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/SimpleConstantBuffer.h"

#include "directX12/DxCommand.h"
#include "directX12/mesh/Mesh.h"
#include "directX12/ShaderManager.h"

#include "directX12/raytracing/RaytracingScene.h"

// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

namespace OriGine {
/// 前方宣言
class ModelMeshRenderer;
class PrimitiveMeshRendererBase;

/// <summary>
/// Texture付きのメッシュ(TextureMesh)を描画するシステム
/// </summary>
class TexturedMeshRenderSystem
    : public BaseRenderSystem {
public:
    TexturedMeshRenderSystem();
    ~TexturedMeshRenderSystem() override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダラーを登録する
    /// </summary>
    /// <param name="_entity">対象のエンティティハンドル</param>
    void DispatchRenderer(EntityHandle _entity) override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// 指定されたブレンドモードでレンダリングを行う
    /// </summary>
    /// <param name="_blendMode">ブレンドモード</param>
    /// <param name="_isCulling">カリングを有効にするかどうか</param>
    void RenderingBy(BlendMode _blendMode, bool _isCulling) override;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    bool ShouldSkipRender() const override;

    /// <summary>
    /// メッシュを描画する
    /// </summary>
    /// <param name="_commandList">コマンドリスト</param>
    /// <param name="_mesh">メッシュデータ</param>
    /// <param name="_transformBuff">トランスフォーム定数バッファ</param>
    /// <param name="_materialBuff">マテリアル定数バッファ</param>
    /// <param name="_textureHandle">テクスチャのGPUハンドル</param>
    void RenderingMesh(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
        const TextureColorMesh& _mesh,
        IConstantBuffer<Transform>& _transformBuff,
        IConstantBuffer<Material>& _materialBuff,
        D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle) const;

    /// <summary>
    /// メッシュを描画する (マテリアルバッファをSimpleConstantBufferで渡す版)
    /// </summary>
    /// <param name="_commandList">コマンドリスト</param>
    /// <param name="_mesh">メッシュデータ</param>
    /// <param name="_transformBuff">トランスフォーム定数バッファ</param>
    /// <param name="_materialBuff">マテリアル定数バッファ</param>
    /// <param name="_textureHandle">テクスチャのGPUハンドル</param>
    void RenderingMesh(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
        const TextureColorMesh& _mesh,
        IConstantBuffer<Transform>& _transformBuff,
        SimpleConstantBuffer<Material>& _materialBuff,
        D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle) const;

    /// <summary>
    /// ModelMeshRendererを描画する
    /// </summary>
    /// <param name="_commandList">コマンドリスト</param>
    /// <param name="_renderer">モデルメッシュレンダラー</param>
    void RenderModelMesh(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, ModelMeshRenderer* _renderer);

    /// <summary>
    /// PrimitiveMeshRendererを描画する
    /// </summary>
    /// <param name="_commandList">コマンドリスト</param>
    /// <param name="_renderer">プリミティブメッシュレンダラー</param>
    void RenderPrimitiveMesh(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, PrimitiveMeshRendererBase* _renderer);

protected:
    /// <summary>
    /// ライトの情報を更新してバインドする
    /// </summary>
    void LightUpdate();

    /// <summary>
    /// レイトレーシングで使用するメッシュが動的かどうかを確認する
    /// </summary>
    /// <param name="_entityHandle">エンティティハンドル</param>
    /// <param name="_type">メッシュのタイプ</param>
    /// <param name="_isModelMesh">モデルメッシュかどうか</param>
    /// <returns>動的であれば true</returns>
    bool MeshIsDynamic(EntityHandle _entityHandle, RaytracingMeshType _type, bool _isModelMesh = false);

    /// <summary>
    /// レイトレーシングシーンの更新
    /// </summary>
    void UpdateRayScene();

protected:
    bool currentCulling_        = true; // 現在のカリング設定
    BlendMode currentBlendMode_ = BlendMode::Alpha; // 現在のブレンドモード

    // アクティブなモデルメッシュレンダラーのリスト [カリングON/OFF][ブレンドモード]
    std::array<std::array<std::vector<ModelMeshRenderer*>, kBlendNum>, 2> activeModelMeshRenderer_{};
    // アクティブなプリミティブメッシュレンダラーのリスト [カリングON/OFF][ブレンドモード]
    std::array<std::array<std::vector<PrimitiveMeshRendererBase*>, kBlendNum>, 2> activePrimitiveMeshRenderer_{};

    // PSOリスト [カリングON/OFF][ブレンドモード]
    std::array<std::array<PipelineStateObj*, kBlendNum>, 2> psoByBlendMode_{};

    std::vector<RaytracingMeshEntry> meshForRaytracing_{}; // レイトレーシング用メッシュのエントリ
    std::vector<RayTracingInstance> rayTracingInstances_{}; // レイトレーシングインスタンス
    std::shared_ptr<RaytracingScene> raytracingScene_ = nullptr; // レイトレーシングシーン

    int32_t transformBufferIndex_          = 0; // トランスフォームバッファのインデックス
    int32_t cameraBufferIndex_             = 0; // カメラバッファのインデックス
    int32_t materialBufferIndex_           = 0; // マテリアルバッファのインデックス
    int32_t directionalLightBufferIndex_   = 0; // 平行光源バッファのインデックス
    int32_t pointLightBufferIndex_         = 0; // 点光源バッファのインデックス
    int32_t spotLightBufferIndex_          = 0; // スポットライトバッファのインデックス
    int32_t lightCountBufferIndex_         = 0; // ライト数バッファのインデックス
    int32_t textureBufferIndex_            = 0; // テクスチャバッファのインデックス
    int32_t environmentTextureBufferIndex_ = 0; // 環境マップテクスチャのインデックス
    int32_t raytracingSceneBufferIndex_    = 0; // レイトレーシングシーンバッファのインデックス

public:
    /// <summary>
    /// ブレンドモードを設定する
    /// </summary>
    /// <param name="_blendMode">ブレンドモード</param>
    void SetBlendMode(BlendMode _blendMode) {
        currentBlendMode_ = _blendMode;
    }
    /// <summary>
    /// カリングの有効・無効を設定する
    /// </summary>
    /// <param name="_isCulling">カリングを有効にするかどうか</param>
    void SetCulling(bool _isCulling) {
        currentCulling_ = _isCulling;
    }
};

} // namespace OriGine
