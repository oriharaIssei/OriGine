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

class SpeedlineEffect
    : public ISystem {
public:
    SpeedlineEffect() : ISystem(SystemCategory::PostRender) {}
    ~SpeedlineEffect() override = default;

    void Initialize() override;
    void Update() override;
    void UpdateEntity(GameEntity* _entity) override;
    void Finalize();

protected:
    void CreatePSO();

    void RenderStart();
    void Render(SpeedlineEffectParam* _param);
    void RenderEnd();

protected:
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;

    std::vector<SpeedlineEffectParam*> activeParams_;
};
