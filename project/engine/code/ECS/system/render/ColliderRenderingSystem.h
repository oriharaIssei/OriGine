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
#include "component/collision/collider/CapsuleCollider.h"
#include "component/collision/collider/OBBCollider.h"
#include "component/collision/collider/RayCollider.h"
#include "component/collision/collider/SegmentCollider.h"
#include "component/collision/collider/SphereCollider.h"

namespace OriGine {

/// <summary>
/// コライダーの形状をデバッグ用に可視化するシステム。
/// AABB, OBB, 球体などのコライダー形状をライン描画する。
/// </summary>
class ColliderRenderingSystem
    : public BaseRenderSystem {
public:
    /// <summary>
    /// デフォルトのメッシュ最大数
    /// </summary>
    static const int32_t kDefaultMeshCount_;

public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    ColliderRenderingSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~ColliderRenderingSystem() override;

    /// <summary>
    /// 初期化処理。各形状用レンダラーの生成を行う。
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新処理。レンダリング対象のクリアと更新を行う。
    /// </summary>
    void Update() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// コライダー描画用のパイプラインステートオブジェクト(PSO)を作成する
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// 現在シーン内の全てのコライダーの形状に基づいて、ラインメッシュ情報を動的に生成する
    /// </summary>
    void CreateRenderMesh();

    /// <summary>
    /// 描画コマンドの発行を行う
    /// </summary>
    void RenderCall();

    /// <summary>
    /// コライダーのレンダリングを統合実行する
    /// </summary>
    void Rendering() override;

    /// <summary>
    /// レンダリングをスキップするかどうかを判定する
    /// </summary>
    /// <returns>true = 描画対象なし / false = 描画対象あり</returns>
    bool ShouldSkipRender() const override;

private:
    /// <summary>
    /// AABBコライダーのコンポーネント配列への参照
    /// </summary>
    ComponentArray<AABBCollider>* aabbColliders_ = nullptr;

    /// <summary>
    /// OBBコライダーのコンポーネント配列への参照
    /// </summary>
    ComponentArray<OBBCollider>* obbColliders_ = nullptr;

    /// <summary>
    /// 球体コライダーのコンポーネント配列への参照
    /// </summary>
    ComponentArray<SphereCollider>* sphereColliders_ = nullptr;

    /// <summary>
    /// AABB描画用ラインレンダラー
    /// </summary>
    std::unique_ptr<LineRenderer> aabbRenderer_;
    std::vector<LineRenderer::MeshType>::iterator aabbMeshItr_;

    /// <summary>
    /// OBB描画用ラインレンダラー
    /// </summary>
    std::unique_ptr<LineRenderer> obbRenderer_;
    std::vector<LineRenderer::MeshType>::iterator obbMeshItr_;

    /// <summary>
    /// 球体描画用ラインレンダラー
    /// </summary>
    std::unique_ptr<LineRenderer> sphereRenderer_;
    std::vector<LineRenderer::MeshType>::iterator sphereMeshItr_;

    /// <summary>
    /// Rayコライダーのコンポーネント配列への参照
    /// </summary>
    ComponentArray<RayCollider>* rayColliders_ = nullptr;

    /// <summary>
    /// Ray描画用ラインレンダラー
    /// </summary>
    std::unique_ptr<LineRenderer> rayRenderer_;
    std::vector<LineRenderer::MeshType>::iterator rayMeshItr_;

    /// <summary>
    /// Segmentコライダーのコンポーネント配列への参照
    /// </summary>
    ComponentArray<SegmentCollider>* segmentColliders_ = nullptr;

    /// <summary>
    /// Segment描画用ラインレンダラー
    /// </summary>
    std::unique_ptr<LineRenderer> segmentRenderer_;
    std::vector<LineRenderer::MeshType>::iterator segmentMeshItr_;

    /// <summary>
    /// Capsuleコライダーのコンポーネント配列への参照
    /// </summary>
    ComponentArray<CapsuleCollider>* capsuleColliders_ = nullptr;

    /// <summary>
    /// Capsule描画用ラインレンダラー
    /// </summary>
    std::unique_ptr<LineRenderer> capsuleRenderer_;
    std::vector<LineRenderer::MeshType>::iterator capsuleMeshItr_;

    /// <summary>
    /// 使用するPSOポインタ
    /// </summary>
    PipelineStateObj* pso_ = nullptr;
};

} // namespace OriGine
