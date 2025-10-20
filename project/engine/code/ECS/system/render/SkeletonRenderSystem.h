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

/// <summary>
/// スケルトンの描画を行うシステム(Debug用)
/// </summary>
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
    /// <summary>
    /// 描画開始処理
    /// </summary>
    void StartRender();
    /// <summary>
    /// 描画処理
    /// </summary>
    void RenderCall();
    /// <summary>
    /// レンダーメッシュの作成
    /// </summary>
    void CreateRenderMesh();

    void CreatePso();

    /// <summary>
    /// 子ジョイントのメッシュを作成
    /// </summary>
    void CreateMeshForChildren(
        std::vector<Mesh<ColorVertexData>>* _jointMeshGroup,
        std::vector<Mesh<ColorVertexData>>* _boneMeshGroup,
        const Matrix4x4& _worldMat,
        const Skeleton& _skeleton, const Joint& _joint, const Joint* _prevJoint, const Vec3f& _prevJointPos);

    /// <summary>
    /// ジョイントメッシュの作成
    /// </summary>
    void CreateJointMesh(
        Mesh<ColorVertexData>* _mesh,
        const Joint& _joint,
        const Vec3f& _center,
        const Vec4f& _color);
    /// <summary>
    /// ボーンメッシュの作成
    /// </summary>
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
