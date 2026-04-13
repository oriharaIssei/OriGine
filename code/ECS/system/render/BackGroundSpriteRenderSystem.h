#pragma once

#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <array>
#include <vector>

/// engine
// directX12Object
#include <directX12/BlendMode.h>
#include <directX12/PipelineStateObj.h>
/// math
#include "Matrix4x4.h"

namespace OriGine {
/// ECS
// component
class SpriteRenderer;

/// <summary>
/// 背景として描画されるスプライトを管理するシステム。
/// ビューポート行列を使用して、スクリーン座標系での描画を行う。
/// </summary>
class BackGroundSpriteRenderSystem
    : public BaseRenderSystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BackGroundSpriteRenderSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BackGroundSpriteRenderSystem() override;

    /// <summary>
    /// 初期化処理。ビューポート行列の計算とPSOの作成を行う。
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// 背景スプライト用のパイプラインステートオブジェクト(PSO)を作成する
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    virtual void StartRender() override;

    /// <summary>
    /// 背景スプライトのレンダリングを実行する。
    /// 描画順序のソートとブレンドモードの制御を行う。
    /// </summary>
    void Rendering() override;

    /// <summary>
    /// エンティティのスプライトレンダラーを登録し、バッファを更新する
    /// </summary>
    /// <param name="_entity">対象のエンティティハンドル</param>
    void DispatchRenderer(EntityHandle _entity) override;

    /// <summary>
    /// レンダリングをスキップするかどうかを判定する。
    /// </summary>
    /// <returns>true = 描画対象なし / false = 描画対象あり</returns>
    bool ShouldSkipRender() const override;

private:
    /// <summary>
    /// 描画対象のレンダラーリスト
    /// </summary>
    std::vector<SpriteRenderer*> renderers_;

    /// <summary>
    /// スクリーン座標系への投影に使用する行列
    /// </summary>
    Matrix4x4 viewPortMat_;

    /// <summary>
    /// ブレンドモードごとのPSO
    /// </summary>
    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};
};

} // namespace OriGine
