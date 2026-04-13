#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// stl
#include <vector>

/// engine
// directX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

/// ECS
// component
#include "component/effect/post/GrayscaleComponent.h"

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

    /// <summary>
    /// ポストエフェクトに使用するコンポーネントを有効な場合にリスト化する等の前処理
    /// </summary>
    /// <param name="_owner">エンティティハンドル</param>
    void DispatchComponent(EntityHandle _owner) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = スキップする / false = スキップしない</returns>
    virtual bool ShouldSkipPostRender() const {
        return grayscaleComps_.empty();
    }

protected:
    PipelineStateObj* pso_ = nullptr;
    std::vector<GrayscaleComponent*> grayscaleComps_; // ポストエフェクトに使用するコンポーネントのハンドルリスト
};

} // namespace OriGine
