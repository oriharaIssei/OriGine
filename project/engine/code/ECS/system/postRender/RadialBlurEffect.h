#pragma once
#include "system/ISystem.h" /// engine

// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

class RadialBlurParam;
class RadialBlurEffect
    : public ISystem {
public:
    RadialBlurEffect() : ISystem(SystemCategory::PostRender) {}
    ~RadialBlurEffect() override = default;

    void Initialize() override;
    void Update() override;
    void UpdateEntity(GameEntity* _entity) override;
    void Finalize();

protected:
    void CreatePSO();

    void RenderStart();
    void Render();

protected:
    std::list<RadialBlurParam*> activeRadialBlurParams_;
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
