#pragma once

/// parent
#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <array>
#include <memory>
#include <vector>

/// engine
// directX12Object
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"

/// ECS/
// component
#include "component/effect/particle/gpuParticle/GpuParticle.h"

namespace OriGine {
/// 前方宣言
// component
class GpuParticleEmitter;

/// <summary>
/// GPUで計算されたパーティクルのレンダリングを行うシステム。
/// 各エミッターからのパーティクルデータを収集し、ブレンドモードごとに描画する。
/// </summary>
class GpuParticleRenderSystem
    : public BaseRenderSystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    GpuParticleRenderSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~GpuParticleRenderSystem() override;

    /// <summary>
    /// 初期化処理。バッファの生成とPSOの作成を行う。
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理。リソースの解放を行う。
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// GPUパーティクル描画用のパイプラインステートオブジェクト(PSO)を作成する
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理。ビュー情報（PerViewバッファ）の更新と設定を行う。
    /// </summary>
    void StartRender() override;

    /// <summary>
    /// 指定されたブレンドモードでGPUパーティクルの描画を実行する
    /// </summary>
    /// <param name="blendMode">ブレンドモード</param>
    /// <param name="_isCulling">カリングを有効にするかどうか</param>
    void RenderingBy(BlendMode _blendMode, bool _isCulling) override;

    /// <summary>
    /// エンティティのGPUパーティクルエミッターを収集する
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
    /// ブレンドモードごとのPSO
    /// </summary>
    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};

    /// <summary>
    /// ブレンドモードごとに分類された、現在アクティブなエミッターのリスト
    /// </summary>
    std::array<std::vector<GpuParticleEmitter*>, kBlendNum> activeEmitterByBlendMode_{};

    /// <summary>
    /// ビュー（カメラ）行列などの情報を保持する定数バッファ
    /// </summary>
    IConstantBuffer<PerView> perViewBuffer_;
};

} // namespace OriGine
