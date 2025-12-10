#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// stl
#include <memory>
#include <vector>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include <component/effect/post/VignetteParam.h>

// component
#include "component/effect/post/SpeedlineEffectParam.h"

namespace OriGine {

/// <summary>
/// スピードラインを描画するシステム
/// </summary>
class SpeedlineEffect
    : public BasePostRenderingSystem {
public:
    SpeedlineEffect();
    ~SpeedlineEffect() override;

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
    /// コンポーネント描画処理
    /// </summary>
    void Render(SpeedlineEffectParam* _param);
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
    void DispatchComponent(Entity* _entity) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    bool ShouldSkipPostRender() const override;

protected:
    PipelineStateObj* pso_                = nullptr;

    std::vector<SpeedlineEffectParam*> activeParams_;
};

} // namespace OriGine
