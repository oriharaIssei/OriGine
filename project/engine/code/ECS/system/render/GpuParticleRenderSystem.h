#pragma once
#include "system/ISystem.h"

/// stl
#include <vector>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/ShaderManager.h"

/// ECS/
// component
#include "component/effect/particle/gpuParticle/GpuParticle.h"

class GpuParticleEmitter;

class GpuParticleRenderSystem
    : public ISystem {
public:
    GpuParticleRenderSystem() : ISystem(SystemCategory::Render) {}
    ~GpuParticleRenderSystem() = default;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePso();

    void StartRender();
    bool isRendering();
    void UpdateEntity(GameEntity* _entity) override;

private:
    BlendMode currentBlend_ = BlendMode::Alpha;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;

    std::vector<GpuParticleEmitter> activeEmitter_;

    IConstantBuffer<PerView> perViewBuffer_;
};
