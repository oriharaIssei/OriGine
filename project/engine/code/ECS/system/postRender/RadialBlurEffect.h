#pragma once
#include "system/ISystem.h" /// engine

// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

class RadialBlurParam;

/// <summary>
/// Radial Blur をかけるシステム
class RadialBlurEffect
    : public ISystem {
public:
    RadialBlurEffect() : ISystem(SystemCategory::PostRender) {}
    ~RadialBlurEffect() override = default;

    void Initialize() override;
    void Update() override;
    void UpdateEntity(Entity* _entity) override;
    void Finalize();

protected:
    void CreatePSO();

    /// <summary>
    /// 描画準備システム
    /// </summary>
    void RenderStart();
    /// <summary>
    /// 描画システム
    /// </summary>
    void Render();

protected:
    std::list<RadialBlurParam*> activeRadialBlurParams_;
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
