#pragma once

/// parent
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
// component
#include "component/collider/Collider.h"
#include "component/renderer/MeshRenderer.h"

class ColliderRenderingSystem
    : public ISystem {
public:
    ColliderRenderingSystem() : ISystem(SystemCategory::Render) {}
    ~ColliderRenderingSystem() {}

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    static const int32_t defaultMeshCount_;

protected:
    void UpdateEntity(GameEntity* /*_entity*/) override;

    void CreateRenderMesh();
    void RenderCall();

    void CreatePso();
    void StartRender();

private:
    ComponentArray<AABBCollider>* aabbColliders_     = nullptr;
    ComponentArray<SphereCollider>* sphereColliders_ = nullptr;

    LineRenderer aabbRenderer_;
    std::vector<Mesh<ColorVertexData>>::iterator aabbMeshItr_;
    LineRenderer sphereRenderer_;
    std::vector<Mesh<ColorVertexData>>::iterator sphereMeshItr_;

    BlendMode currentBlend_ = BlendMode::Alpha;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
};
