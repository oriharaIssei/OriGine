#pragma once
#include "../ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
// component
class SpriteRenderer;
/// math
#include "Matrix4x4.h"

class SpriteRenderSystem
    : public ISystem {
public:
    SpriteRenderSystem() : ISystem(SystemType::Render) {};
    ~SpriteRenderSystem() = default;
    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePso();
    void StartRender();
    void UpdateEntity(GameEntity* _entity) override;

private:
    std::vector<SpriteRenderer*> renderers_;
    Matrix4x4 viewPortMat_;

    BlendMode currentBlend_ = BlendMode::Alpha;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
};
