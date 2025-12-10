#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

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

namespace OriGine {

/// <summary>
/// スケルトンの描画を行うシステム(Debug用)
/// </summary>
class SkeletonRenderSystem
    : public BaseRenderSystem {
public:
    SkeletonRenderSystem();
    ~SkeletonRenderSystem() override;

    void Initialize() override;
    void Finalize() override;

    static const int32_t defaultMeshCount_;

protected:
    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// レンダリング処理(StartRenderから描画まですべてを行う)
    /// </summary>
    void Rendering() override;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    bool ShouldSkipRender() const override;

    /// <summary>
    /// 描画処理
    /// </summary>
    void RenderCall();

    /// <summary>
    /// レンダーメッシュの作成
    /// </summary>
    void CreateRenderMesh();

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

    std::unique_ptr<LineRenderer> jointRenderer_;
    std::vector<Mesh<ColorVertexData>>::iterator jointMeshItr_;
    std::unique_ptr<LineRenderer> boneRenderer_;
    std::vector<Mesh<ColorVertexData>>::iterator boneMeshItr_;

    BlendMode currentBlend_ = BlendMode::Alpha;

    PipelineStateObj* pso_ = nullptr;
};

} // namespace OriGine
