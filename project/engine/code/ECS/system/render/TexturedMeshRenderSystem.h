#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/IConstantBuffer.h"
#include "directX12/SimpleConstantBuffer.h"

#include "directX12/DxCommand.h"
#include "directX12/Mesh.h"
#include "directX12/ShaderManager.h"
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

class ModelMeshRenderer;
class PrimitiveMeshRendererBase;

class TexturedMeshRenderSystem
    : public ISystem {
public:
    TexturedMeshRenderSystem();
    ~TexturedMeshRenderSystem();

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    void CreatePso();

    void StartRender();

    void UpdateEntity(GameEntity* _entity) override;

    void RenderingMesh(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
        const TextureMesh& _mesh,
        IConstantBuffer<Transform>& _transformBuff,
        IConstantBuffer<Material>& _materialBuff,
        D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle) const;

    void RenderingMesh(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
        const TextureMesh& _mesh,
        IConstantBuffer<Transform>& _transformBuff,
        SimpleConstantBuffer<Material>& _materialBuff,
        D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle) const;

    void RenderModelMesh(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, ModelMeshRenderer* _renderer);
    void RenderPrimitiveMesh(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, PrimitiveMeshRendererBase* _renderer);

    void SettingPSO(BlendMode _blend);

protected:
    void LightUpdate();

    void DispatchRenderer(GameEntity* _entity);
    void RenderingBy(BlendMode _blendMode);

private:
    std::unordered_map<BlendMode, std::vector<ModelMeshRenderer*>> activeModelMeshRenderer_;
    std::unordered_map<BlendMode, std::vector<PrimitiveMeshRendererBase*>> activePrimitiveMeshRenderer_;

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
