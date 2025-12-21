#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// stl
#include <array>
#include <memory>
#include <vector>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

// component
#include "component/effect/post/RandomEffectParam.h"

namespace OriGine {

/// <summary>
/// ランダムエフェクトシステム
/// </summary>
class RandomEffect
    : public BasePostRenderingSystem {
public:
    RandomEffect();
    ~RandomEffect() override;

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
    void DispatchComponent(EntityHandle _handle) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    bool ShouldSkipPostRender() const override;

protected:
    std::array<PipelineStateObj*, static_cast<size_t>(kBlendNum)> psoByBlendMode_                       = {};
    std::vector<RandomEffectParam*> activeParams_ = {};
    BlendMode currentBlend_                                                                             = BlendMode::Alpha;
};

} // namespace OriGine
