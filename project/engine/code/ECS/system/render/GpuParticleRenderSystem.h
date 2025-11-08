#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <array>
#include <memory>
#include <vector>

/// engine
// directX12Object
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"

/// ECS/
// component
#include "component/effect/particle/gpuParticle/GpuParticle.h"

class GpuParticleEmitter;

/// <summary>
/// Gpuパーティクル描画システム
/// </summary>
class GpuParticleRenderSystem
    : public BaseRenderSystem {
public:
    GpuParticleRenderSystem();
    ~GpuParticleRenderSystem() override;

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
    void RenderingBy(BlendMode _blendMode, bool _isCulling) override;

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity"></param>
    void DispatchRenderer(Entity* _entity) override;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    bool ShouldSkipRender() const override;

private:
    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};

    std::array<std::vector<GpuParticleEmitter*>, kBlendNum> activeEmitterByBlendMode_{};

    IConstantBuffer<PerView> perViewBuffer_;
};
