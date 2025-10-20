#pragma once
#include "../ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
// component
class SpriteRenderer;
/// math
#include "Matrix4x4.h"

/// <summary>
/// 背景スプライトの描画を行うシステム
/// </summary>
class BackGroundSpriteRenderSystem
    : public ISystem {
public:
    BackGroundSpriteRenderSystem() : ISystem(SystemCategory::Render) {};
    ~BackGroundSpriteRenderSystem() = default;
    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePso();
    void StartRender();
    /// <summary>
    /// BlendModeごとに描画を行う
    /// </summary>
    /// <param name="blendMode"></param>
    void RenderingBy(BlendMode blendMode);

    void UpdateEntity(Entity* _entity) override;

private:
    std::unordered_map<BlendMode, std::vector<SpriteRenderer*>> renderersByBlend_;
    Matrix4x4 viewPortMat_;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
};
