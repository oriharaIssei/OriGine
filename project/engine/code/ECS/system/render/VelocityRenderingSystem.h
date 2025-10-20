#pragma once

#include "component/renderer/MeshRenderer.h"
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
#include "system/ISystem.h"

// Rigidbody / Transform
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

/// <summary>
/// Velocityを描画するシステム
/// </summary>
class VelocityRenderingSystem : public ISystem {
public:
    VelocityRenderingSystem() : ISystem(SystemCategory::Render) {}
    ~VelocityRenderingSystem() {}

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    static const int32_t defaultMeshCount_;

private:
    /// <summary>
    /// Velocityの情報を元に描画用メッシュを作成
    /// </summary>
    void CreateRenderMesh();
    /// <summary>
    /// 描画開始処理
    /// </summary>
    void StartRender();
    /// <summary>
    /// 描画コール
    /// </summary>
    void RenderCall();
    void CreatePso();

private:
    ComponentArray<Rigidbody>* rigidbodies_ = nullptr;

    LineRenderer velocityRenderer_;
    std::vector<Mesh<ColorVertexData>>::iterator velocityMeshItr_;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    PipelineStateObj* pso_;
};
