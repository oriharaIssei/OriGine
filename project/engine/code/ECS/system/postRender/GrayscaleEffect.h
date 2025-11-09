#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// engine
// directX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

/// <summary>
/// グレースケール を画面全体にかけるエフェクト
/// </summary>
class GrayscaleEffect
    : public BasePostRenderingSystem {
public:
    GrayscaleEffect();
    ~GrayscaleEffect() override;

    void Initialize() override;
    void Finalize();

protected:
    /// <summary>
    /// PSO作成
    /// </summary>
    void CreatePSO()override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void RenderStart()override;
    /// <summary>
    /// レンダリング処理
    /// </summary>
    void Rendering()override;
    /// <summary>
    /// レンダリング終了処理
    /// </summary>
    void RenderEnd()override;

protected:
    PipelineStateObj* pso_                = nullptr;
};
