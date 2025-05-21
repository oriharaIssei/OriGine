#pragma once
#include "system/ISystem.h"

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

class OutlineRender
    : public ISystem {
public:
    OutlineRender() : ISystem(SystemType::PostRender) {}
    ~OutlineRender() override = default;

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
