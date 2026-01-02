#pragma once

#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <memory>

/// engine
// directX12Object
#include "directX12/DxCommand.h"

/// ECS
// component
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"
#include "component/renderer/MeshRenderer.h"

namespace OriGine {

/// <summary>
/// Light のデバッグ描画システム
/// </summary>
class LightDebugRenderingSystem
    : public BaseRenderSystem {
private:
    static const uint32_t kDefaultMeshCount_;

public:
    LightDebugRenderingSystem();
    ~LightDebugRenderingSystem() override;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void CreatePSO() override;
    void StartRender() override;
    void Rendering() override;
    bool ShouldSkipRender() const override;

private:
    void CreateRenderMesh();
    void RenderCall();

private:
    ComponentArray<PointLight>* pointLights_ = nullptr;
    ComponentArray<SpotLight>* spotLights_   = nullptr;

    std::unique_ptr<LineRenderer> pointRenderer_;
    std::vector<LineRenderer::MeshType>::iterator pointLightMeshItr_;
    std::unique_ptr<LineRenderer> spotRenderer_;
    std::vector<LineRenderer::MeshType>::iterator spotLightMeshItr_;

    PipelineStateObj* pso_ = nullptr;
};

} // namespace OriGine
