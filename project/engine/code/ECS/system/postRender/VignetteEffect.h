#pragma once
#include "system/ISystem.h"

///stl
#include <memory>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include <component/effect/post/VignetteParam.h>

/// <summary>
/// ヴィネットエフェクト
/// </summary>
class VignetteEffect
    : public ISystem {
public:
    VignetteEffect() : ISystem(SystemCategory::PostRender) {}
    ~VignetteEffect() override = default;

    void Initialize() override;
    void Update() override;
    void UpdateEntity(Entity* _entity) override;
    void Finalize();

protected:
    void CreatePSO();

    /// <summary>
    /// 描画準備処理
    /// </summary>
    void RenderStart();
    /// <summary>
    /// 描画
    /// </summary>
    void Render();
    /// <summary>
    /// 描画終了処理
    /// </summary>
    void RenderEnd();

protected:
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
