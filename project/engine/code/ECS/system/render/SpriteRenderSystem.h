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
/// Spriteの描画を行うシステム
/// </summary>
class SpriteRenderSystem
    : public ISystem {
public:
    SpriteRenderSystem() : ISystem(SystemCategory::Render) {};
    ~SpriteRenderSystem() = default;
    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePso();
    /// <summary>
    /// 描画開始処理
    /// </summary>
    void StartRender();
    void UpdateEntity(Entity* _entity) override;

    /// <summary>
    /// 描画情報によって振り分ける
    /// </summary>
    void DispatchRenderer(Entity* _entity);

    /// <summary>
    /// ブレンドモードごとに描画を行う
    /// </summary>
    void RenderingBy(BlendMode _blendMode);

private:
    std::unordered_map<BlendMode, std::vector<SpriteRenderer*>> renderers_;
    Matrix4x4 viewPortMat_;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
};
