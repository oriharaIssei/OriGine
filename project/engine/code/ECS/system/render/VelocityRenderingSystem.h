#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
// component
#include "component/renderer/MeshRenderer.h"
// Rigidbody / Transform
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

/// <summary>
/// Velocityを描画するシステム
/// </summary>
class VelocityRenderingSystem
    : public BaseRenderSystem {
public:
    static const int32_t defaultMeshCount_;
public:
    VelocityRenderingSystem();
    ~VelocityRenderingSystem();

    void Initialize() override;
    void Finalize() override;

private:
    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO()override;

    /// <summary>
    /// 描画開始処理
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// 描画コール
    /// </summary>
    void RenderCall();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Rendering()override;

    /// <summary>
    /// 描画スキップ判定
    /// </summary>
    bool IsSkipRendering() const override;

    /// <summary>
    /// Velocityの情報を元に描画用メッシュを作成
    /// </summary>
    void CreateRenderMesh();
private:
    PipelineStateObj* pso_;
    ComponentArray<Rigidbody>* rigidbodies_ = nullptr;

    std::unique_ptr<LineRenderer> velocityRenderer_;
    std::vector<Mesh<ColorVertexData>>::iterator velocityMeshItr_;
};
