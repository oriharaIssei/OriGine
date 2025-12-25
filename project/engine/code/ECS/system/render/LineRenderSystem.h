#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <array>
#include <memory>

/// component
#include "component/renderer/MeshRenderer.h"

/// engine
// directX12Object
#include "directX12/BlendMode.h"
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"

namespace OriGine {

/// <summary>
/// 線の描画を行うシステム
/// </summary>
class LineRenderSystem
    : public BaseRenderSystem {
public:
    LineRenderSystem();
    ~LineRenderSystem() override;

    void Initialize() override;
    void Finalize() override;

    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// BlendModeごとに描画を行う
    /// </summary>
    /// <param name="blendMode">ブレンドモード</param>
    void RenderingBy(BlendMode _blendMode, bool _isCulling) override;

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity"></param>
    void DispatchRenderer(EntityHandle _entity) override;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    bool ShouldSkipRender() const override;

private:
    BlendMode currentBlendMode_ = BlendMode::Alpha;
    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};
    std::array<std::vector<LineRenderer*>, kBlendNum> activeLineRenderersByBlendMode_{};

public:
    const std::array<PipelineStateObj*, kBlendNum>& GetPsoByBlendMode() {
        return psoByBlendMode_;
    }

    BlendMode GetCurrentBlendMode() const {
        return currentBlendMode_;
    }
    void SetBlendMode(BlendMode _blendMode) {
        currentBlendMode_ = _blendMode;
    }
};

} // namespace OriGine
