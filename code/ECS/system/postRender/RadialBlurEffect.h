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
/// </summary>
class RadialBlurEffect
    : public BasePostRenderingSystem {
public:
    RadialBlurEffect();
    ~RadialBlurEffect() override;

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
    void DispatchComponent(const EntityHandle& _handle) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    bool ShouldSkipPostRender() const override;

protected:
    std::vector<RadialBlurParam*> activeRadialBlurParams_ = {}; // 今回の更新で描画対象となったパラメータ一覧
    PipelineStateObj* pso_                                = nullptr; // ラジアルブラーエフェクト用PSO
};

} // namespace OriGine
