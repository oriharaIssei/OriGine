#pragma once
#include "system/ISystem.h"

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include <component/effect/post/VignetteParam.h>

/// <summary>
/// Dissolveエフェクトシステム
/// </summary>
class DissolveEffect
    : public ISystem {
public:
    DissolveEffect() : ISystem(SystemCategory::PostRender) {}
    ~DissolveEffect() override = default;

    void Initialize() override;
    void Update() override;
    void UpdateEntity(Entity* _entity) override;
    void Finalize();

    /// <summary>
    /// 単一エフェクトに対してエフェクトをかけ, RenderTextureに出力する
    /// </summary>
    void EffectEntity(RenderTexture* _output, Entity* _entity);

protected:
    void CreatePSO();

    /// <summary>
    /// レンダリング開始
    /// </summary>
    void RenderStart();
    /// <summary>
    /// _viewHandle で指定されたテクスチャに対してエフェクトの描画を行う
    /// </summary>
    void Render(D3D12_GPU_DESCRIPTOR_HANDLE _viewHandle);

protected:
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
