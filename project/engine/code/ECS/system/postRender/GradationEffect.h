#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// stl
#include <vector>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

// component
#include "component/effect/post/GradationTextureComponent.h"

/// <summary>
/// テクスチャを使用してグラデーションエフェクトをかけるシステム
/// </summary>
class GradationEffect
    : public BasePostRenderingSystem {
public:
    GradationEffect();
    ~GradationEffect() override;

    void Initialize() override;
    void Finalize();

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
    /// レンダリング処理
    /// </summary>
    void Rendering() override;
    /// <summary>
    /// レンダリング終了処理
    /// </summary>
    void RenderEnd() override;

    /// <summary>
    /// PostEffectに使用するComponentを登録する
    /// </summary>
    void DispatchComponent(Entity* _entity) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    bool ShouldSkipPostRender() const override;

protected:
    struct RenderingData {
        GradationTextureComponent* effectParam = nullptr;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle  = D3D12_GPU_DESCRIPTOR_HANDLE(0);
    };

protected:
    PipelineStateObj* pso_ = nullptr;

    std::vector<RenderingData> activeRenderingData_ = {};
};
