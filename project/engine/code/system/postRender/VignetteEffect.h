#pragma once
#include "system/ISystem.h"

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

class VignetteEffect
    : public ISystem {
public:
    VignetteEffect() : ISystem(SystemType::PostRender) {}
    ~VignetteEffect() override = default;

    void Initialize() override;
    void Update() override;
    void Finalize();

protected:
    void CreatePSO();

    void Render();

protected:
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
