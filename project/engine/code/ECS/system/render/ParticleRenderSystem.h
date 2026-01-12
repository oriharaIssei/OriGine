#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <array>
#include <memory>
#include <vector>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"

// component
#include "component/effect/particle/emitter/Emitter.h"

namespace OriGine {

/// <summary>
/// パーティクル描画システム
/// </summary>
class ParticleRenderSystem
    : public BaseRenderSystem {
public:
    ParticleRenderSystem();
    ~ParticleRenderSystem() override;

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
    /// BlendModeごとに描画を行う
    /// </summary>
    /// <param name="blendMode">ブレンドモード</param>
    /// <param name="_isCulling">カリングの有効化</param>
    void RenderingBy(BlendMode /*_blendMode*/, bool /*_isCulling*/) override;

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity"></param>
    void DispatchRenderer(EntityHandle /*_owner*/) override;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    bool ShouldSkipRender() const;

    /// <summary>
    /// 使用していない
    /// </summary>
    /// <param name=""></param>
    void UpdateEntity(EntityHandle /*_owner*/) override {}

private:
    std::array<std::vector<Emitter*>, kBlendNum> activeEmittersByBlendMode_{};

    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};
};

} // namespace OriGine
