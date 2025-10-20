#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>
#include <vector>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"

// component
#include "component/effect/particle/emitter/Emitter.h"

/// <summary>
/// パーティクル描画システム
/// </summary>
class ParticleRenderSystem
    : public ISystem {
public:
    ParticleRenderSystem() : ISystem(SystemCategory::Render) {}
    ~ParticleRenderSystem() = default;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePso();

    /// <summary>
    /// レンダー開始処理
    /// </summary>
    void StartRender();
    /// <summary>
    /// 描画するものを振り分ける
    /// </summary>
    void DispatchRenderer(Entity* _entity);
    /// <summary>
    /// ブレンドモードごとに描画処理を行う
    /// </summary>
    void RenderingBy(BlendMode _blend);

    /// <summary>
    /// 使用していない
    /// </summary>
    /// <param name=""></param>
    void UpdateEntity(Entity* /*_entity*/) override{}

private:
    std::unordered_map<BlendMode, std::vector<Emitter*>> activeEmittersByBlendMode_;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
};
