#pragma once

#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"

class LineRenderSystem
    : public ISystem {
public:
    LineRenderSystem() : ISystem(SystemCategory::Render) {}
    ~LineRenderSystem() {}

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(GameEntity* _entity) override;

    void CreatePso();
    void StartRender();

private:
    BlendMode currentBlend_               = BlendMode::Alpha;
    bool lineIsStrip_                     = false;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
};
