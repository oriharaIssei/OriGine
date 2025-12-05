#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/buffer/ConstantBuffer.h"
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
    : public BaseRenderSystem {
public:
    TexturedMeshRenderSystem();
    ~TexturedMeshRenderSystem() override;

    void Initialize() override;
    void Finalize() override;

    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダラーをディスパッチする
    /// </summary>
    void DispatchRenderer(Entity* _entity) override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// ブレンドモードごとにレンダリングを行う
    /// </summary>
    void RenderingBy(BlendMode _blendMode, bool _isCulling) override;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    bool ShouldSkipRender() const override;

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
        ConstantBuffer<Transform>& _transformBuff,
        ConstantBuffer<Material>& _materialBuff,
        D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle) const;

    /// <summary>
    /// メッシュを描画する (マテリアルバッファをSimpleConstantBufferで渡す版)
    /// </summary>
    void RenderingMesh(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
        const TextureMesh& _mesh,
        ConstantBuffer<Transform>& _transformBuff,
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

protected:
    void LightUpdate();

private:
    bool currentCulling_        = true;
    BlendMode currentBlendMode_ = BlendMode::Alpha;
    // value : { non Culling配列 , Culling配列}
    std::array<std::array<std::vector<ModelMeshRenderer*>, kBlendNum>, 2> activeModelMeshRenderer_{};
    std::array<std::array<std::vector<PrimitiveMeshRendererBase*>, kBlendNum>, 2> activePrimitiveMeshRenderer_{};

    // value : { non Culling , Culling }
    std::array<std::array<PipelineStateObj*, kBlendNum>, 2> psoByBlendMode_{};

    int32_t transformBufferIndex_          = 0;
    int32_t cameraBufferIndex_             = 0;
    int32_t materialBufferIndex_           = 0;
    int32_t directionalLightBufferIndex_   = 0;
    int32_t pointLightBufferIndex_         = 0;
    int32_t spotLightBufferIndex_          = 0;
    int32_t lightCountBufferIndex_         = 0;
    int32_t textureBufferIndex_            = 0;
    int32_t environmentTextureBufferIndex_ = 0;

public:
    void SetBlendMode(BlendMode _blendMode) {
        currentBlendMode_ = _blendMode;
    }
    void SetCulling(bool _isCulling) {
        currentCulling_ = _isCulling;
    }
};
