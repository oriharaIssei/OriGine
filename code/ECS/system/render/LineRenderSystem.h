#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <array>
#include <memory>

/// engine
// directX12Object
#include "directX12/BlendMode.h"
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"

/// component
#include "component/renderer/LineRenderer.h"

namespace OriGine {

/// <summary>
/// ライン（線）の描画を行うシステム。
/// LineRendererコンポーネントを持つエンティティを収集し、ブレンドモードごとに描画する。
/// </summary>
class LineRenderSystem
    : public BaseRenderSystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    LineRenderSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~LineRenderSystem() override;

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// ライン描画用のパイプラインステートオブジェクト(PSO)を作成する
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理。ビュー情報の設定などを行う。
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// 指定されたブレンドモードでラインの描画を実行する
    /// </summary>
    /// <param name="blendMode">ブレンドモード</param>
    /// <param name="_isCulling">カリングを有効にするかどうか</param>
    void RenderingBy(BlendMode _blendMode, bool _isCulling) override;

    /// <summary>
    /// エンティティのラインレンダラーを登録し、トランスフォームを更新する
    /// </summary>
    /// <param name="_entity">対象のエンティティハンドル</param>
    void DispatchRenderer(EntityHandle _entity) override;

    /// <summary>
    /// レンダリングをスキップするかどうかを判定する
    /// </summary>
    /// <returns>true = 描画対象なし / false = 描画対象あり</returns>
    bool ShouldSkipRender() const override;

private:
    /// <summary>
    /// 現在のブレンドモード
    /// </summary>
    BlendMode currentBlendMode_ = BlendMode::Alpha;

    /// <summary>
    /// ブレンドモードごとのPSO
    /// </summary>
    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};

    /// <summary>
    /// ブレンドモードごとに分類された、現在アクティブなラインレンダラーのリスト
    /// </summary>
    std::array<std::vector<LineRenderer*>, kBlendNum> activeLineRenderersByBlendMode_{};

public:
    /// <summary>
    /// ブレンドモードごとのPSOリストを取得する
    /// </summary>
    /// <returns>PSOリスト</returns>
    const std::array<PipelineStateObj*, kBlendNum>& GetPsoByBlendMode() {
        return psoByBlendMode_;
    }

    /// <summary>
    /// 現在のブレンドモードを取得する
    /// </summary>
    /// <returns>ブレンドモード</returns>
    BlendMode GetCurrentBlendMode() const {
        return currentBlendMode_;
    }

    /// <summary>
    /// ブレンドモードを設定する
    /// </summary>
    /// <param name="_blendMode">設定するブレンドモード</param>
    void SetBlendMode(BlendMode _blendMode) {
        currentBlendMode_ = _blendMode;
    }
};

} // namespace OriGine
