#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// stl
#include <memory>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

namespace OriGine {
/// engine
// drecitX12
class RenderTexture;
/// ECS
// component
struct Transform;
class DistortionEffectParam;
// system
class PrimitiveMeshRendererBase;
class TexturedMeshRenderSystemWithoutRaytracing;

/// <summary>
/// 歪みエフェクトをかけるシステム
/// </summary>
class DistortionEffect
    : public BasePostRenderingSystem {
public:
    DistortionEffect();
    ~DistortionEffect() override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// PSO作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void RenderStart() override;
    /// <summary>
    /// レンダリング処理(RenderStart,RenderEndは呼び出さない。)
    /// </summary>
    void Rendering() override;
    /// <summary>
    /// レンダリング終了処理
    /// </summary>
    void RenderEnd() override;

    /// <summary>
    /// PostEffectに使用するComponentを登録する
    /// (Systemによっては使用しない)
    /// </summary>
    void DispatchComponent(const EntityHandle& _handle) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    bool ShouldSkipPostRender() const override;

protected:
    /// <summary>
    /// 1エンティティ分の描画に必要なデータ
    /// </summary>
    struct RenderingData {
        DistortionEffectParam* effectParam    = nullptr;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = D3D12_GPU_DESCRIPTOR_HANDLE(0);
    };

protected:
    std::unique_ptr<DistortionEffectParam> defaultParam_ = nullptr; // デフォルトパラメーター (3DObjectの適応に使う)

    std::vector<RenderingData> activeRenderingData_                  = {}; // 今回の更新で描画対象となったエンティティの描画データ一覧
    std::vector<PrimitiveMeshRendererBase*> activeDistortionObjects_ = {}; // 歪みを適用する3Dオブジェクトの一覧

    // 静的共有リソース
    static std::unique_ptr<RenderTexture> distortionSceneTexture_; // 歪みシーンテクスチャ (全インスタンスで共有)
    static int32_t instanceCount_; // インスタンスカウント

    std::unique_ptr<TexturedMeshRenderSystemWithoutRaytracing> texturedMeshRenderSystem_ = nullptr; // 歪みシーンテクスチャへの描画に使うメッシュ描画システム
    PipelineStateObj* pso_                                                               = nullptr; // 歪みエフェクト用PSO

    // gpu に送る バッファーのインデックス
    int32_t distortionTextureIndex_ = 0;
    int32_t sceneTextureIndex_      = 0;
    int32_t distortionParamIndex_   = 0;
    int32_t materialIndex_          = 0;
};

} // namespace OriGine
