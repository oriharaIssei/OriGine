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
    LineRenderSystem() : ISystem(SystemType::Render) {}
    ~LineRenderSystem() {}

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    void UpdateEntity(GameEntity* _entity) override;
    void StartRender();

private:
    void CreatePso();

private:
    BlendMode currentBlend_ = BlendMode::Alpha;
    bool lineIsStrip_       = false;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;

public:
    DxCommand* getDxCommand() {
        return dxCommand_.get();
    }
    const std::unordered_map<BlendMode, PipelineStateObj*>& getPso() {
        return pso_;
    }
};
