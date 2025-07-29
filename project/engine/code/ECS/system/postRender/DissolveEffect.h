#pragma once
#include "system/ISystem.h"

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include <component/effect/post/VignetteParam.h>

class DissolveEffect
    : public ISystem {
public:
    DissolveEffect() : ISystem(SystemType::PostRender) {}
    ~DissolveEffect() override = default;

    void Initialize() override;
    void Update() override;
    void UpdateEntity(GameEntity* _entity) override;
    void Finalize();

protected:
    void CreatePSO();

    void RenderStart();
    void Render();

protected:
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
