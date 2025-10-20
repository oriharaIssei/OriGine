#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/SimpleConstantBuffer.h"

#include "directX12/DxCommand.h"
#include "directX12/mesh/Mesh.h"
#include "directX12/ShaderManager.h"
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

class ModelMeshRenderer;
class PrimitiveMeshRendererBase;

/// <summary>
/// Texture付きのメッシュ(TextureMesh)を描画するシステム
/// </summary>
class TexturedMeshRenderSystem
    : public ISystem {
public:
    TexturedMeshRenderSystem();
    ~TexturedMeshRenderSystem();

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    void CreatePso();

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void StartRender();

    void UpdateEntity(Entity* _entity) override;

    /// <summary>
    /// メッシュを描画する
    /// </summary>
    /// <param name="_commandList"></param>
    /// <param name="_mesh"></param>
    /// <param name="_transformBuff"></param>
    /// <param name="_materialBuff"></param>
    /// <param name="_textureHandle"></param>
    void RenderingMesh(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
        const TextureMesh& _mesh,
        IConstantBuffer<Transform>& _transformBuff,
        IConstantBuffer<Material>& _materialBuff,
        D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle) const;

    /// <summary>
    /// メッシュを描画する (マテリアルバッファをSimpleConstantBufferで渡す版)
    /// </summary>
    void RenderingMesh(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
        const TextureMesh& _mesh,
        IConstantBuffer<Transform>& _transformBuff,
        SimpleConstantBuffer<Material>& _materialBuff,
        D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle) const;

    /// <summary>
    /// ModelMeshRendererを描画する
    /// </summary>
    void RenderModelMesh(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, ModelMeshRenderer* _renderer);
    /// <summary>
    /// PrimitiveMeshRendererを描画する
    /// </summary>
    void RenderPrimitiveMesh(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, PrimitiveMeshRendererBase* _renderer);

    void SettingPSO(BlendMode _blend);

protected:
    void LightUpdate();

    /// <summary>
    /// レンダラーをディスパッチする
    /// </summary>
    void DispatchRenderer(Entity* _entity);
    /// <summary>
    /// ブレンドモードごとにレンダリングを行う
    /// </summary>
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
