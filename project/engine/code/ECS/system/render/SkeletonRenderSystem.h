#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
// component
#include "component/animation/SkinningAnimationComponent.h"
#include "component/ComponentArray.h"
#include "component/renderer/MeshRenderer.h"

class SkeletonRenderSystem
    : public ISystem {
public:
    SkeletonRenderSystem();
    ~SkeletonRenderSystem() override;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    static const int32_t defaultMeshCount_;

protected:
    void RenderCall();
    void CreateRenderMesh();
    void CreatePso();
    void StartRender();

    void CreateMeshForChildren(
        std::vector<Mesh<ColorVertexData>>* _jointMeshGroup,
        std::vector<Mesh<ColorVertexData>>* _boneMeshGroup,
        const Matrix4x4& _worldMat,
        const Skeleton& _skeleton, const Joint& _joint, const Joint* _prevJoint, const Vec3f& _prevJointPos);

    void CreateJointMesh(
        Mesh<ColorVertexData>* _mesh,
        const Joint& _joint,
        const Vec3f& _center,
        const Vec4f& _color);
    void CreateBoneMesh(
        Mesh<ColorVertexData>* _mesh,
        const Vec3f& _start,
        const Vec3f& _end,
        const Vec4f& _color);

private:
    ComponentArray<SkinningAnimationComponent>* skinningAnimationArray_ = nullptr;

    LineRenderer jointRenderer_;
    std::vector<Mesh<ColorVertexData>>::iterator jointMeshItr_;
    LineRenderer boneRenderer_;
    std::vector<Mesh<ColorVertexData>>::iterator boneMeshItr_;

    BlendMode currentBlend_ = BlendMode::Alpha;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;
};
