#pragma once

#include "base/BasePostRenderingSystem.h"

/// stl
#include <list>
#include <vector>

/// engine
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"

/// ECS
// component
#include "component/effect/post/OutlineComponent.h"
#include "component/renderer/ModelMeshRenderer.h"
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"

namespace OriGine {

/// <summary>
/// 背面法 アウトラインレンダリングシステム
/// </summary>
class OutlineRenderSystem
    : public BasePostRenderingSystem {
public:
    OutlineRenderSystem();
    ~OutlineRenderSystem() override;

    void Initialize() override;
    void Finalize() override;

private:
    /// <summary>
    /// PSO(パイプラインステートオブジェクト)の作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理（シェーダーパラメータセット等）
    /// </summary>
    virtual void RenderStart() override;

    /// <summary>
    /// レンダリング処理（描画命令の発行）
    /// </summary>
    virtual void Rendering() override;

    /// <summary>
    /// レンダリング終了処理（リソースのバリア変更等）
    /// </summary>
    virtual void RenderEnd() override;

    /// <summary>
    /// ポストエフェクトに使用するコンポーネントを有効な場合にリスト化する等の前処理
    /// </summary>
    /// <param name="_entity">エンティティハンドル</param>
    virtual void DispatchComponent(EntityHandle _entity);

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = スキップする / false = スキップしない</returns>
    virtual bool ShouldSkipPostRender() const;

private:
    /// <summary>
    /// アウトライン描画用エントリー
    /// </summary>
    struct OutlineEntry {
        OutlineComponent* outlineComp = nullptr;
        std::list<ModelMeshRenderer*> modelMeshRenderers;
        std::list<PrimitiveMeshRendererBase*> primitiveMeshRenderers;
    };

private:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr; // DXコマンド
    PipelineStateObj* pso_                = nullptr;

    // アクティブなモデルメッシュレンダラーのリスト
    std::vector<OutlineEntry> activeEntries_{};

    int32_t transformBufferIndex_    = 0; // トランスフォームバッファのインデックス
    int32_t cameraBufferIndex_       = 0; // カメラバッファのインデックス
    int32_t materialBufferIndex_     = 0; // マテリアルバッファのインデックス
    int32_t textureBufferIndex_      = 0; // テクスチャバッファのインデックス
    int32_t outlineParamBufferIndex_ = 0; // アウトラインパラメータバッファのインデックス
};

} // namespace OriGine
