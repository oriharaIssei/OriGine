#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <array>
#include <memory>

/// engine
// directX12
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
// component
#include "component/renderer/SkyboxRenderer.h"

namespace OriGine {

/// <summary>
/// Skybox描画システム
/// </summary>
class SkyboxRender
    : public BaseRenderSystem {
public:
    SkyboxRender();
    ~SkyboxRender() override;

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
    /// PSOの作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity">対象のエンティティハンドル</param>
    void DispatchRenderer(EntityHandle _entity) override;

    /// <summary>
    /// BlendModeごとに描画を行う
    /// </summary>
    /// <param name="_blendMode">ブレンドモード</param>
    /// <param name="_isCulling">カリングを有効にするかどうか</param>
    void RenderingBy(BlendMode _blendMode, bool _isCulling) override;

    /// <summary>
    /// レンダリングをスキップするかどうかを判定する
    /// </summary>
    /// <returns>true = 描画対象なし / false = 描画対象あり</returns>
    bool ShouldSkipRender() const override;

private:
    /// <summary>
    /// ブレンドモードごとの PSO
    /// </summary>
    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};

    /// <summary>
    /// ブレンドモードごとのアクティブなレンダラーリスト
    /// </summary>
    std::array<std::vector<SkyboxRenderer*>, kBlendNum> rendererByBlendMode_ = {};
};

} // namespace OriGine
