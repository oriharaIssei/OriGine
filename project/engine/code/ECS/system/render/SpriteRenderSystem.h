#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
/// math
#include "Matrix4x4.h"

namespace OriGine {
/// 前方宣言
// component
class SpriteRenderer;

/// <summary>
/// Spriteの描画を行うシステム
/// </summary>
class SpriteRenderSystem
    : public BaseRenderSystem {
public:
    SpriteRenderSystem();
    ~SpriteRenderSystem() override;

    void Initialize() override;
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
    /// BlendModeごとに描画を行う
    /// </summary>
    /// <param name="blendMode">ブレンドモード</param>
    /// <param name="_isCulling">カリングの有効化</param>
    void Rendering() override;

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
    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};

    Matrix4x4 viewPortMat_;
    std::vector<SpriteRenderer*> renderers_;
};

} // namespace OriGine
