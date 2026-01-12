#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// engine
// directX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

namespace OriGine {

/// <summary>
/// グレースケール を画面全体にかけるエフェクト
/// </summary>
class GrayscaleEffect
    : public BasePostRenderingSystem {
public:
    GrayscaleEffect();
    ~GrayscaleEffect() override;

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
    /// レンダリング処理
    /// </summary>
    void Rendering() override;
    /// <summary>
    /// レンダリング終了処理
    /// </summary>
    void RenderEnd() override;

protected:
    PipelineStateObj* pso_ = nullptr;
};

} // namespace OriGine
