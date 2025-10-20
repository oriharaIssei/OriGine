#pragma once

#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// component
#include "component/renderer/MeshRenderer.h"

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"

/// <summary>
/// 線の描画を行うシステム
/// </summary>
class LineRenderSystem
    : public ISystem {
public:
    LineRenderSystem() : ISystem(SystemCategory::Render) {}
    ~LineRenderSystem() {}

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    void UpdateEntity(Entity* _entity) override;
    void StartRender();
    void settingPSO(BlendMode _blend);
private:
    void CreatePso();

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity"></param>
    void DispatchRenderer(Entity* _entity);
    /// <summary>
    /// ブレンドモードごとに描画
    /// </summary> 
    void RenderingBy(BlendMode _blend);

private:
    bool lineIsStrip_       = false;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
    std::unordered_map<BlendMode, std::vector<LineRenderer>> activeLineRenderersByBlendMode_;

public:
    DxCommand* getDxCommand() {
        return dxCommand_.get();
    }
    const std::unordered_map<BlendMode, PipelineStateObj*>& getPso() {
        return pso_;
    }
};
