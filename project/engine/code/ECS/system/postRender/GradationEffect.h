#pragma once
#include "system/ISystem.h"

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include <component/effect/post/VignetteParam.h>

/// <summary>
/// テクスチャを使用してグラデーションエフェクトをかけるシステム
/// </summary>
class GradationEffect
    : public ISystem {
public:
    GradationEffect() : ISystem(SystemCategory::PostRender) {}
    ~GradationEffect() override = default;

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
    /// 描画開始処理
    /// </summary>
    void RenderStart();
    /// <summary>
    /// エンティティのコンポーネントをセットアップして描画する
    /// </summary>
    void SetupComponentAndRender(Entity* _entity, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, D3D12_GPU_DESCRIPTOR_HANDLE _viewHandle);
    /// <summary>
    /// _viewHandleで指定されたテクスチャに対してエフェクトを掛ける
    /// </summary>
    void Render(D3D12_GPU_DESCRIPTOR_HANDLE _viewHandle);

protected:
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
