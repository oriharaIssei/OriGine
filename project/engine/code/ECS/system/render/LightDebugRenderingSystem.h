#pragma once

#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <memory>

/// engine
// directX12Object
#include "directX12/DxCommand.h"

/// ECS
// component
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"
#include "component/renderer/MeshRenderer.h"

namespace OriGine {

/// <summary>
/// ポイントライトやスポットライトの範囲をデバッグ用に可視化するシステム。
/// 各ライトの影響範囲をライン描画する。
/// </summary>
class LightDebugRenderingSystem
    : public BaseRenderSystem {
private:
    /// <summary>
    /// デフォルトのメッシュ最大数
    /// </summary>
    static const uint32_t kDefaultMeshCount_;

public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    LightDebugRenderingSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~LightDebugRenderingSystem() override;

    /// <summary>
    /// 初期化処理。ポイントライトとスポットライト用のレンダラー生成を行う。
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
    /// ライトデバッグ描画用のパイプラインステートオブジェクト(PSO)を作成する
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理。ビュー情報の設定などを行う。
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// ライトデバッグ情報のレンダリングを統合実行する
    /// </summary>
    void Rendering() override;

    /// <summary>
    /// レンダリングをスキップするかどうかを判定する
    /// </summary>
    /// <returns>true = 描画対象なし / false = 描画対象あり</returns>
    bool ShouldSkipRender() const override;

private:
    /// <summary>
    /// 現在シーン内の全てのライトに基づき、デバッグ用ラインメッシュを動的に生成する
    /// </summary>
    void CreateRenderMesh();

    /// <summary>
    /// 描画コマンドの発行を行う
    /// </summary>
    void RenderCall();

private:
    /// <summary>
    /// ポイントライトのコンポーネント配列への参照
    /// </summary>
    ComponentArray<PointLight>* pointLights_ = nullptr;

    /// <summary>
    /// スポットライトのコンポーネント配列への参照
    /// </summary>
    ComponentArray<SpotLight>* spotLights_ = nullptr;

    /// <summary>
    /// ポイントライト描画用ラインレンダラー
    /// </summary>
    std::unique_ptr<LineRenderer> pointRenderer_;
    std::vector<LineRenderer::MeshType>::iterator pointLightMeshItr_;

    /// <summary>
    /// スポットライト描画用ラインレンダラー
    /// </summary>
    std::unique_ptr<LineRenderer> spotRenderer_;
    std::vector<LineRenderer::MeshType>::iterator spotLightMeshItr_;

    /// <summary>
    /// 使用するPSOポインタ
    /// </summary>
    PipelineStateObj* pso_ = nullptr;
};

} // namespace OriGine
