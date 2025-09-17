#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>
#include <vector>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"

// component
#include "component/effect/particle/emitter/Emitter.h"

class ParticleRenderSystem
    : public ISystem {
public:
    ParticleRenderSystem() : ISystem(SystemCategory::Render) {}
    ~ParticleRenderSystem() = default;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePso();

    void StartRender();
    void DispatchRenderer(GameEntity* _entity);
    void RenderingBy(BlendMode _blend);

    /// <summary>
    /// 使用していない
    /// </summary>
    /// <param name=""></param>
    void UpdateEntity(GameEntity* /*_entity*/) override{}

private:
    std::unordered_map<BlendMode, std::vector<Emitter*>> activeEmittersByBlendMode_;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
};
