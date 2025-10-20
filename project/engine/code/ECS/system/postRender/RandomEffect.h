#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

/// <summary>
/// ランダムエフェクトシステム
/// </summary>
class RandomEffect
    : public ISystem {
public:
    RandomEffect() : ISystem(SystemCategory::PostRender) {}
    ~RandomEffect() override = default;

    void Initialize() override;
    void Update() override;
    void UpdateEntity(Entity* _entity) override;
    void Finalize();

protected:
    void CreatePSO();

    /// <summary>
    /// 描画準備
    /// </summary>
    void RenderStart();
    void Render();

    /// <summary>
    /// ブレンドモード変更
    /// </summary>
    void ChangeBlendMode(BlendMode mode);

protected:
    BlendMode currentBlend_ = BlendMode::Alpha;

    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
