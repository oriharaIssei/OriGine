#pragma once
#include "../ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"

class TexturedMeshRenderSystem
    : public ISystem {
public:
    TexturedMeshRenderSystem() : ISystem(SystemType::Render) {};
    ~TexturedMeshRenderSystem() = default;    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePso();

    void LightUpdate();
    void StartRender();

    void UpdateEntity(GameEntity* _entity) override;

private:
    BlendMode currentBlend_ = BlendMode::Alpha;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;

    int32_t transformBufferIndex_          = 0;
    int32_t cameraBufferIndex_             = 0;
    int32_t materialBufferIndex_           = 0;
    int32_t directionalLightBufferIndex_   = 0;
    int32_t pointLightBufferIndex_         = 0;
    int32_t spotLightBufferIndex_          = 0;
    int32_t lightCountBufferIndex_         = 0;
    int32_t textureBufferIndex_            = 0;
    int32_t environmentTextureBufferIndex_ = 0;
};
