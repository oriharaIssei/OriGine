#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// stl
#include <vector>

/// engine
// drecitX12
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

// component
#include "component/effect/post/SmoothingEffectParam.h"

namespace OriGine {

/// <summary>
/// SmoothingEffect をかけるシステム
/// </summary>
class SmoothingEffect
    : public BasePostRenderingSystem {
public:
    SmoothingEffect();
    ~SmoothingEffect() override;

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
    PipelineStateObj* pso_ = nullptr;
    std::vector<SmoothingEffectParam*> activeParams_;
};

} // namespace OriGine
