#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>
#include <vector>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/Mesh.h"
#include "directX12/ShaderManager.h"
// component
#include "component/animation/SkinningAnimationComponent.h"
#include "component/material/Material.h"
#include "component/renderer/MeshRenderer.h"
#include "component/transform/Transform.h"

class SkinningAnimationComponent;
class ModelMeshRenderer;

class SkinningMeshRenderSystem
    : public ISystem {
public:
    SkinningMeshRenderSystem();
    ~SkinningMeshRenderSystem();

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    void CreatePso();

    void StartRender();

    void UpdateEntity(GameEntity* _entity) override;

    void RenderModelMesh(
        Transform* _entityTransform,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
        SkinningAnimationComponent* _skinningAnimationComponent,
        ModelMeshRenderer* _renderer);

protected:
    void LightUpdate();

    void DispatchRenderer(GameEntity* _entity);
    void RenderingBy(BlendMode _blendMode);

private:
    struct RenderingData {
        SkinningAnimationComponent* _skinningAnimationComponent;
        ModelMeshRenderer* _renderer;
        Transform* _entityTransform;
    };
    std::unordered_map<BlendMode, std::vector<RenderingData>> activeRenderersByBlendMode_;
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
