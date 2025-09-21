#pragma once

#include "component/renderer/MeshRenderer.h"
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
#include "system/ISystem.h"

// Rigidbody / Transform
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

class VelocityRenderingSystem : public ISystem {
public:
    VelocityRenderingSystem() : ISystem(SystemCategory::Render) {}
    ~VelocityRenderingSystem() {}

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    static const int32_t defaultMeshCount_;

private:
    void CreateRenderMesh();
    void RenderCall();
    void CreatePso();
    void StartRender();

private:
    ComponentArray<Rigidbody>* rigidbodies_ = nullptr;

    LineRenderer velocityRenderer_;
    std::vector<Mesh<ColorVertexData>>::iterator velocityMeshItr_;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    PipelineStateObj* pso_;
};
