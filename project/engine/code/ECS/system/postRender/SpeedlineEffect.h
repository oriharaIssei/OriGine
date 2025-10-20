#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <vector>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include <component/effect/post/VignetteParam.h>

// component
#include "component/effect/post/SpeedlineEffectParam.h"

/// <summary>
/// スピードラインを描画するシステム
/// </summary>
class SpeedlineEffect
    : public ISystem {
public:
    SpeedlineEffect() : ISystem(SystemCategory::PostRender) {}
    ~SpeedlineEffect() override = default;

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
    /// コンポーネント描画処理
    /// </summary>
    void Render(SpeedlineEffectParam* _param);
    /// <summary>
    /// 描画終了処理
    /// </summary>
    void RenderEnd();

protected:
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;

    std::vector<SpeedlineEffectParam*> activeParams_;
};
