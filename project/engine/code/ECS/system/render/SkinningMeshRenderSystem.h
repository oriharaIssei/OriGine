#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <array>
#include <memory>
#include <vector>

/// engine
// directX12Object
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/DxCommand.h"
#include "directX12/mesh/Mesh.h"
#include "directX12/ShaderManager.h"
// component
#include "component/animation/SkinningAnimationComponent.h"
#include "component/material/Material.h"
#include "component/renderer/MeshRenderer.h"
#include "component/transform/Transform.h"

namespace OriGine {
/// 前方宣言
class SkinningAnimationComponent;
class ModelMeshRenderer;

/// <summary>
/// スキニングメッシュレンダリングシステム
/// </summary>
class SkinningMeshRenderSystem
    : public BaseRenderSystem {
public:
    SkinningMeshRenderSystem();
    ~SkinningMeshRenderSystem() override;

    void Initialize() override;
    void Finalize() override;

    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// BlendModeごとに描画を行う
    /// </summary>
    /// <param name="blendMode">ブレンドモード</param>
    /// <param name="_isCulling">カリングの有効化</param>
    void RenderingBy(BlendMode _blendMode, bool _isCulling) override;

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity"></param>
    void DispatchRenderer(Entity* _entity) override;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    bool ShouldSkipRender() const override;

    /// <summary>
    /// ModelMeshのレンダリング
    /// </summary>
    /// <param name="_entityTransform"></param>
    /// <param name="_commandList"></param>
    /// <param name="_skinningAnimationComponent"></param>
    /// <param name="_renderer"></param>
    void RenderModelMesh(
        Transform* _entityTransform,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
        SkinningAnimationComponent* _skinningAnimationComponent,
        ModelMeshRenderer* _renderer);

protected:
    void LightUpdate();

private:
    struct RenderingData {
        SkinningAnimationComponent* _skinningAnimationComponent;
        ModelMeshRenderer* _renderer;
        Transform* _entityTransform;
    };

private:
    std::array<std::vector<RenderingData>, kBlendNum> activeRenderersByBlendMode_{};
    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};

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

} // namespace OriGine
