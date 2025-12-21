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
class TexturedMeshRenderSystem;

/// <summary>
/// 歪みエフェクトをかけるシステム
/// </summary>
class DistortionEffect
    : public BasePostRenderingSystem {
public:
    DistortionEffect();
    ~DistortionEffect() override;

    void Initialize() override;
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
    void DispatchComponent(EntityHandle _handle) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    bool ShouldSkipPostRender() const override;

protected:
    struct RenderingData {
        DistortionEffectParam* effectParam    = nullptr;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = D3D12_GPU_DESCRIPTOR_HANDLE(0);
    };

protected:
    std::unique_ptr<DistortionEffectParam> defaultParam_   = nullptr; // デフォルトパラメーター (3DObjectの適応に使う)
    std::unique_ptr<RenderTexture> distortionSceneTexture_ = nullptr;

    std::vector<RenderingData> activeRenderingData_                  = {};
    std::vector<PrimitiveMeshRendererBase*> activeDistortionObjects_ = {};

    std::unique_ptr<TexturedMeshRenderSystem> texturedMeshRenderSystem_ = nullptr;
    PipelineStateObj* pso_                                              = nullptr;

    // gpu に送る バッファーのインデックス
    int32_t distortionTextureIndex_ = 0;
    int32_t sceneTextureIndex_      = 0;
    int32_t distortionParamIndex_   = 0;
    int32_t materialIndex_          = 0;
};

} // namespace OriGine
