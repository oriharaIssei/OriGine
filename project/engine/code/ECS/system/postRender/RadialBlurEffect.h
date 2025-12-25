#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

namespace OriGine {
/// ECS
// component
class RadialBlurParam;

/// <summary>
/// Radial Blur をかけるシステム
class RadialBlurEffect
    : public BasePostRenderingSystem {
public:
    RadialBlurEffect();
    ~RadialBlurEffect() override;

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
    /// (Systemによっては使用しない)
    /// </summary>
    void DispatchComponent(EntityHandle _handle) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    bool ShouldSkipPostRender() const override;

protected:
    std::vector<RadialBlurParam*> activeRadialBlurParams_ = {};
    PipelineStateObj* pso_                                = nullptr;
};

} // namespace OriGine
