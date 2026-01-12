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

namespace OriGine {

/// <summary>
/// Velocityを描画するシステム
/// </summary>
class VelocityRenderingSystem
    : public BaseRenderSystem {
public:
    static const int32_t kDefaultMeshCount_;

public:
    VelocityRenderingSystem();
    ~VelocityRenderingSystem() override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

private:
    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// 描画の発行
    /// </summary>
    void RenderCall();

    /// <summary>
    /// レンダリング実行
    /// </summary>
    void Rendering() override;

    /// <summary>
    /// レンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = スキップする</returns>
    bool ShouldSkipRender() const override;

    /// <summary>
    /// 速度(Velocity)の情報を元に描画用メッシュを作成する
    /// </summary>
    void CreateRenderMesh();

private:
    PipelineStateObj* pso_                  = nullptr; // パイプラインステートオブジェクト
    ComponentArray<Rigidbody>* rigidbodies_ = nullptr; // リジッドボディコンポーネント配列へのポインタ

    std::unique_ptr<LineRenderer> velocityRenderer_; // 速度描画用レンダラー
    std::vector<Mesh<ColorVertexData>>::iterator velocityMeshItr_; // メッシュイテレータ
};

} // namespace OriGine
