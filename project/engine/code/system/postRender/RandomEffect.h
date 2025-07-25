#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include <component/effect/post/VignetteParam.h>

class RandomEffect
    : public ISystem {
public:
    RandomEffect() : ISystem(SystemType::PostRender) {}
    ~RandomEffect() override = default;

    void Initialize() override;
    void Update() override;
    void UpdateEntity(GameEntity* _entity) override;
    void Finalize();

protected:
    void CreatePSO();

    void RenderStart();
    void Render();

    void ChangeBlendMode(BlendMode mode);

protected:
    BlendMode currentBlend_ = BlendMode::Alpha;

    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
