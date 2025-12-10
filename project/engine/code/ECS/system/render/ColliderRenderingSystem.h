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
#include "component/collision/collider/Collider.h"
#include "component/renderer/MeshRenderer.h"
// colliderType
#include "component/collision/collider/AABBCollider.h"
#include "component/collision/collider/OBBCollider.h"
#include "component/collision/collider/SphereCollider.h"

namespace OriGine {

/// <summary>
/// Colliderのレンダリングを行うシステム(Debug用)
/// </summary>
class ColliderRenderingSystem
    : public BaseRenderSystem {
public:
    static const int32_t kDefaultMeshCount_;
public:
    ColliderRenderingSystem();
    ~ColliderRenderingSystem() override;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

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
    /// 当たり判定の形状にメッシュを作成
    /// </summary>
    void CreateRenderMesh();

    /// <summary>
    /// 描画コマンドの呼び出し
    /// </summary>
    void RenderCall();

    /// <summary>
    /// レンダリング処理(StartRenderから描画まですべてを行う)
    /// </summary>
    void Rendering() override;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    bool ShouldSkipRender() const override;

private:
    ComponentArray<AABBCollider>* aabbColliders_     = nullptr;
    ComponentArray<OBBCollider>* obbColliders_       = nullptr;
    ComponentArray<SphereCollider>* sphereColliders_ = nullptr;

    std::unique_ptr<LineRenderer> aabbRenderer_;
    std::vector<LineRenderer::MeshType>::iterator aabbMeshItr_;
    std::unique_ptr<LineRenderer> obbRenderer_;
    std::vector<LineRenderer::MeshType>::iterator obbMeshItr_;
    std::unique_ptr<LineRenderer> sphereRenderer_;
    std::vector<LineRenderer::MeshType>::iterator sphereMeshItr_;

    PipelineStateObj* pso_                = nullptr;
};

} // namespace OriGine
