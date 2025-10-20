#pragma once

#include "system/ISystem.h"

/// stl
#include <memory>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
class RenderTexture;

/// ECS
struct Transform;
class DistortionEffectParam;

class TexturedMeshRenderSystem;

/// <summary>
/// 歪みエフェクトをかけるシステム
/// </summary>
class DistortionEffect
    : public ISystem {
public:
    DistortionEffect();
    ~DistortionEffect() override;

    void Initialize() override;
    void Update() override;
    void Finalize();

    void UpdateEntity(Entity* _entity) override;

    /// <summary>
    /// 単一エフェクトに対してエフェクトをかけ, RenderTextureに出力する
    /// </summary>
    void EffectEntity(RenderTexture* _output, Entity* _entity);

protected:
    /// <summary>
    /// コンポーネントの持つエフェクトオブジェクトのシーンを描画する
    /// </summary>
    void RenderEffectObjectScene(
        const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList,
        Transform* _entityTransform,
        DistortionEffectParam* _param);

    void CreatePSO();

protected:
    std::unique_ptr<RenderTexture> distortionSceneTexture_ = nullptr;

    //! TODO : 専用の描画を用意する.
    std::unique_ptr<TexturedMeshRenderSystem> texturedMeshRenderSystem_ = nullptr;
    PipelineStateObj* pso_                                              = nullptr;
    std::unique_ptr<DxCommand> dxCommand_                               = nullptr;

    // gpu に送る バッファーのインデックス
    int32_t distortionTextureIndex_ = 0;
    int32_t sceneTextureIndex_      = 0;
    int32_t distortionParamIndex_   = 0;
    int32_t materialIndex_          = 0;
};
