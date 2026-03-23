#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <memory>

/// engine
// directX12
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
// renderer
#include "component/renderer/LineRenderer.h"
// component arrays
#include "component/effect/particle/emitter/ParticleSystem.h"
#include "component/spawner/EntitySpawner.h"

namespace OriGine {

/// <summary>
/// ParticleSystem および EntitySpawner が持つ Emitter の EmitterShape を
/// デバッグ用にライン描画するシステム。
/// ColliderRenderingSystem と同じ PSO・描画フローを使用する。
/// </summary>
class EmitterShapeRenderingSystem
    : public BaseRenderSystem {
public:
    static const int32_t kDefaultMeshCount_;

public:
    EmitterShapeRenderingSystem();
    ~EmitterShapeRenderingSystem() override;

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

    /// <summary>
    /// Emitter の EmitterShape を適切なレンダラーへ追加する。
    /// Emitter と EntitySpawner の両方から呼ばれる。
    /// </summary>
    void AddShapeToMesh(const Emitter& _ctrl);

private:
    ComponentArray<ParticleSystem>* emitters_ = nullptr;
    ComponentArray<EntitySpawner>* spawners_  = nullptr;

    // 形状ごとのレンダラー
    std::unique_ptr<LineRenderer> sphereRenderer_;
    std::unique_ptr<LineRenderer> boxRenderer_;
    std::unique_ptr<LineRenderer> capsuleRenderer_;
    std::unique_ptr<LineRenderer> coneRenderer_;

    // 書き込み位置イテレータ
    std::vector<LineRenderer::MeshType>::iterator sphereMeshItr_;
    std::vector<LineRenderer::MeshType>::iterator boxMeshItr_;
    std::vector<LineRenderer::MeshType>::iterator capsuleMeshItr_;
    std::vector<LineRenderer::MeshType>::iterator coneMeshItr_;

    PipelineStateObj* pso_ = nullptr;
};

} // namespace OriGine
