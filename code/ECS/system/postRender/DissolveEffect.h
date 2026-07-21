#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

// component
#include "component/effect/post/DissolveEffectParam.h"

namespace OriGine {

/// <summary>
/// Dissolveエフェクトシステム
/// </summary>
class DissolveEffect
    : public BasePostRenderingSystem {
public:
    DissolveEffect();
    ~DissolveEffect() override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
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
    void DispatchComponent(const EntityHandle& _handle) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    bool ShouldSkipPostRender() const override;

    /// <summary>
    /// _viewHandle で指定されたテクスチャに対してエフェクトの描画を行う
    /// </summary>
    void RenderCall(D3D12_GPU_DESCRIPTOR_HANDLE _viewHandle);

protected:
    /// <summary>
    /// 1エンティティ分の描画に必要なデータ
    /// </summary>
    struct RenderingData {
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = {}; // 描画対象テクスチャのSRVハンドル
        DissolveEffectParam* dissolveParam    = nullptr; // Dissolveパラメータへのポインタ
    };

protected:
    PipelineStateObj* pso_                          = nullptr; // Dissolveエフェクト用PSO
    std::vector<RenderingData> activeRenderingData_ = {}; // 今回の更新で描画対象となったエンティティの描画データ一覧
};

} // namespace OriGine
