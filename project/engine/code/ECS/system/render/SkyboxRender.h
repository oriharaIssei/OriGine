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

/// <summary>
/// Skybox描画システム
/// </summary>
class SkyboxRender
    : public BaseRenderSystem {
public:
    SkyboxRender();
    ~SkyboxRender() override;

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
    /// レンダリング処理(StartRenderから描画まですべてを行う)
    /// </summary>
    void Rendering() override;

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity"></param>
    void DispatchRenderer(Entity* _entity) override;

    /// <summary>
    /// BlendModeごとに描画を行う
    /// </summary>
    void RenderingBy(BlendMode _blendMode, bool _isCulling) override;

    bool IsSkipRendering() const override;

private:
    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};
    std::array<std::vector<SkyboxRenderer*>, kBlendNum> rendererByBlendMode_ = {};
};
