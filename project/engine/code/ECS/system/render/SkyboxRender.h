#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"

class SkyboxRender
    : public ISystem {
public:
    SkyboxRender() : ISystem(SystemCategory::Render) {};
    ~SkyboxRender() = default;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePso();

    void StartRender();

    void UpdateEntity(GameEntity* _entity) override;

private:
    BlendMode currentBlend_ = BlendMode::Alpha;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
};
